/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "../job_tree.h"
#include <unordered_set>

namespace ecs {
namespace impl {

inline JobNode::~JobNode () {
    if (job) {
        delete job;
        job = nullptr;
    }
}

inline JobTree::~JobTree () {
    if (nodeMemory) {
        delete[] nodeMemory;
        nodeMemory = nullptr;
    }
}

struct JobDependencyData {
    JobNode * node = nullptr;
    JobId id = UINT32_MAX;
    bool isGrouped = false;
    bool isSorted = false;
    std::unordered_set<JobNode*> hardDeps; // Writes and RUN_BEFORE/AFTER
    std::unordered_set<JobNode*> outReadDeps;
    std::unordered_set<JobNode*> inReadDeps;
    std::unordered_set<JobDependencyData*> orderDeps;
};

struct DependencyGroup {
    ComponentFlags read;
    ComponentFlags write;
    std::vector<JobDependencyData*> jobs;
    std::vector<JobDependencyData*> sortedJobs;

    std::unordered_set<JobNode*> hardDeps;
    std::unordered_set<DependencyGroup*> outReadDeps;
    std::unordered_set<DependencyGroup*> inReadDeps;
};

struct LowestNode {
    JobNode * node;
    uint32_t depth;
};

template<typename T>
inline void JobTree::ForEachNodeInternal (JobNode * node, T func) const {
    func(node);
    for (auto dep : node->dependents)
        ForEachNodeInternal(dep, func);
}

template<typename T>
inline void JobTree::ForEachNode (T func) const {
    for (auto node : topNodes)
        ForEachNodeInternal(node, func);
}

inline void JobTree::AddToDependencyGroup (JobDependencyData * data, DependencyGroup * group) {
    data->isGrouped = true;
    group->read.SetFlags(data->node->job->GetReadFlags());
    group->write.SetFlags(data->node->job->GetWriteFlags());
    group->jobs.push_back(data);

    for (auto node : data->hardDeps)
        group->hardDeps.emplace(node);
}

inline void JobTree::BuildHardDependencyGroups (std::vector<DependencyGroup> & groups, std::vector<JobDependencyData> & depData) {
    for (auto i = 0; i < depData.size(); ++i) {
        auto & depI = depData[i];
        if (depI.isGrouped)
            continue;

        DependencyGroup group;
        AddToDependencyGroup(&depI, &group);

        for (auto j = i + 1; j < depData.size(); ++j) {
            auto & depJ = depData[j];
            if (depJ.isGrouped)
                continue;

            if (group.hardDeps.find(depJ.node) != group.hardDeps.end()) {
                AddToDependencyGroup(&depJ, &group);

                // Reset our iterator since we might need to group other jobs that didn't previously
                j = i;
            }
        }

        // Stage 3-4: Topological sort group by order specs, preferring incoming read counts
        while (group.jobs.size() > group.sortedJobs.size()) {
            JobDependencyData * bestJob = nullptr;
            for (auto job : group.jobs) {
                if (job->isSorted)
                    continue;
                if (bestJob && bestJob->inReadDeps.size() >= job->inReadDeps.size())
                    continue;
                bool waitingOnOrderDep = false;
                for (auto orderDep : job->orderDeps) {
                    if (orderDep->isSorted)
                        continue;
                    waitingOnOrderDep = true;
                    break;
                }
                if (waitingOnOrderDep)
                    continue;
                bestJob = job;
            }
            assert(bestJob != nullptr); // This means that there is a circular depedency in RUN_THIS_BEFORE/AFTER

            bestJob->isSorted = true;
            group.sortedJobs.push_back(bestJob);
        }
        for (auto j = 0; j < group.sortedJobs.size() - 1; ++j)
            group.sortedJobs[j]->node->dependents.push_back(group.sortedJobs[j+1]->node);

        groups.push_back(std::move(group));
    }
}

inline void JobTree::BuildJobDependencyData (std::vector<JobDependencyData> & depData) {
    for (auto i = 0; i < depData.size(); ++i) {
        auto & depI = depData[i];
        auto & readI = depI.node->job->GetReadFlags();
        auto & writeI = depI.node->job->GetWriteFlags();
        auto & runAfterI = depI.node->job->GetRunAfter();
        auto & runBeforeI = depI.node->job->GetRunBefore();

        for (auto j = i + 1; j < depData.size(); ++j) {
            auto & depJ = depData[j];
            auto & readJ = depJ.node->job->GetReadFlags();
            auto & writeJ = depJ.node->job->GetWriteFlags();
            auto & runAfterJ = depJ.node->job->GetRunAfter();
            auto & runBeforeJ = depJ.node->job->GetRunBefore();

            bool iAfterJ = runAfterI.find(depJ.id) != runAfterI.end() || runBeforeJ.find(depI.id) != runBeforeJ.end();
            bool jAfterI = runBeforeI.find(depJ.id) != runBeforeI.end() || runAfterJ.find(depI.id) != runAfterJ.end();

            bool writeMatch = writeI.HasAny(writeJ);
            bool circularRead = writeI.HasAny(readJ) && writeJ.HasAny(readI);
            bool explicitOrder = iAfterJ || jAfterI;
            if (writeMatch || circularRead || explicitOrder) {
                depI.hardDeps.emplace(depJ.node);
                depJ.hardDeps.emplace(depI.node);
                if (iAfterJ)
                    depI.orderDeps.emplace(&depJ);
                if (jAfterI)
                    depJ.orderDeps.emplace(&depI);
            }
            else if (writeI.HasAny(readJ)) {
                depI.inReadDeps.emplace(depJ.node);
                depJ.outReadDeps.emplace(depI.node);
            }
            else if (writeJ.HasAny(readI)) {
                depJ.inReadDeps.emplace(depI.node);
                depI.outReadDeps.emplace(depJ.node);
            }
        }
    }
}

inline void JobTree::FindLowestSatisfyingNode (JobNode * node, uint32_t depth, const ComponentFlags & flags, LowestNode & results) {
    if (results.depth < depth) {
        if (node->job->GetWriteFlags().HasAny(flags)) {
            results.node = node;
            results.depth = depth;
        }
    }
    for (auto dependent : node->dependents)
        FindLowestSatisfyingNode(dependent, depth + 1, flags, results);
}

template<typename T>
inline JobTree * JobTree::New () {
    // Stage 1: Gather job dependency data
    // Stage 2: Group by hard dependencies (write collision, circular read, before/after)
    // Stage 3: Build dependency graph for each group with run before/after as the deps
    // Stage 4: Topological sort groups, preferring nodes that have the most incoming reads first
    //          Note: Circular dependency in this stage is a user created failure and should assert
    // Stage 5: Build dependency graph of groups with reads as the dependencies
    // Stage 6: Topological sort dependency graph preferring largest group size, followed by incoming reads
    //          Note: Circular dependency in this stage should be allowed

    std::vector<UpdateGroupJob> & updateGroupJobs = GetUpdateGroupJobs<T>();

    JobTree * tree = new JobTree();
    tree->nodeMemory = new JobNode[updateGroupJobs.size()];

    std::vector<JobDependencyData> depData;
    for (auto i = 0; i < updateGroupJobs.size(); ++i) {
        depData.push_back(JobDependencyData{ tree->nodeMemory + i, updateGroupJobs[i].id });
        tree->nodeMemory[i].job = updateGroupJobs[i].factory();
    }

    // Stage 1: Gather job dependency data
    BuildJobDependencyData(depData);

    // Stage 2-4: Build hard dependency groups that will need to run in serial
    std::vector<DependencyGroup> hardGroups;
    BuildHardDependencyGroups(hardGroups, depData);

    // Build new dependency graph using these combined hard depedencies as nodes, and reads on them as dependencies
    // TODO: everything past this point is wrong

    // Sort groups by longest chain
    std::sort(hardGroups.begin(), hardGroups.end(), [](const DependencyGroup & a, const DependencyGroup & b) {
        return a.jobs.size() > b.jobs.size();
    });

    // Insert groups into the tree at the lowest node that satisfies their combined read dependencies
    for (auto & group : hardGroups) {
        LowestNode results;
        results.depth = 0;
        results.node = nullptr;

        for (auto topNode : tree->topNodes)
            FindLowestSatisfyingNode(topNode, 1, group.read, results);

        JobNode * currentNode = results.node;
        for (auto jobData : group.jobs) {
            if (currentNode)
                currentNode->dependents.push_back(jobData->node);
            else
                tree->topNodes.push_back(jobData->node);

            currentNode = jobData->node;
        }
    }

    // TODO: Sort children by depth (this mostly happens by above sorts, but not 100%)

    return tree;
}

} // namespace impl
} // namespace ecs
