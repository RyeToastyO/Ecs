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
    std::vector<JobDependencyData*> jobs;
    std::vector<JobDependencyData*> sortedJobs;

    ComponentFlags read;
    ComponentFlags write;

    bool isSorted = false;

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

inline void JobTree::BuildGroupDependencyData (std::vector<DependencyGroup> & groups) {
    for (size_t i = 0; i < groups.size(); ++i) {
        auto & groupI = groups[i];

        for (size_t j = i + 1; j < groups.size(); ++j) {
            auto & groupJ = groups[j];

            if (groupI.read.HasAny(groupJ.write)) {
                groupI.outReadDeps.emplace(&groupJ);
                groupJ.inReadDeps.emplace(&groupI);
            }
            if (groupJ.read.HasAny(groupI.write)) {
                groupJ.outReadDeps.emplace(&groupI);
                groupI.inReadDeps.emplace(&groupJ);
            }
        }
    }
}

inline void JobTree::BuildHardDependencyGroups (std::vector<DependencyGroup> & groups, std::vector<JobDependencyData> & depData) {
    for (size_t i = 0; i < depData.size(); ++i) {
        auto & depI = depData[i];
        if (depI.isGrouped)
            continue;

        DependencyGroup group;
        AddToDependencyGroup(&depI, &group);

        for (size_t j = i + 1; j < depData.size(); ++j) {
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
        for (size_t j = 0; j < group.sortedJobs.size() - 1; ++j)
            group.sortedJobs[j]->node->dependents.push_back(group.sortedJobs[j+1]->node);

        groups.push_back(std::move(group));
    }
}

inline void JobTree::BuildJobDependencyData (std::vector<JobDependencyData> & depData) {
    for (size_t i = 0; i < depData.size(); ++i) {
        auto & depI = depData[i];
        auto & readI = depI.node->job->GetReadFlags();
        auto & writeI = depI.node->job->GetWriteFlags();
        auto & runAfterI = depI.node->job->GetRunAfter();
        auto & runBeforeI = depI.node->job->GetRunBefore();

        for (size_t j = i + 1; j < depData.size(); ++j) {
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

inline void JobTree::FindLowestSatisfyingNode (JobNode * node, uint32_t depth, const ComponentFlags & read, const ComponentFlags & write, LowestNode & results) {
    if (results.depth < depth) {
        if (node->job->GetWriteFlags().HasAny(read) || node->job->GetReadFlags().HasAny(write)) {
            results.node = node;
            results.depth = depth;
        }
    }
    for (auto dependent : node->dependents)
        FindLowestSatisfyingNode(dependent, depth + 1, read, write, results);
}

inline void JobTree::InsertIntoTree (const std::vector<DependencyGroup*> & groups) {
    for (auto & group : groups) {
        LowestNode results;
        results.depth = 0;
        results.node = nullptr;

        for (auto topNode : topNodes)
            FindLowestSatisfyingNode(topNode, 1, group->read, group->write, results);

        JobNode * toInsert = group->sortedJobs[0]->node;
        if (results.node)
            results.node->dependents.push_back(toInsert);
        else
            topNodes.push_back(toInsert);
    }
}

inline void JobTree::SortDependencyGroups (std::vector<DependencyGroup> & groups, std::vector<DependencyGroup*> & sortedGroups) {
    while (groups.size() > sortedGroups.size()) {
        DependencyGroup * bestGroup = nullptr;
        uint32_t bestIncompleteDeps = UINT32_MAX;
        for (auto & group : groups) {
            if (group.isSorted)
                continue;
            uint32_t incompleteDeps = 0;
            for (auto outRead : group.outReadDeps) {
                if (outRead->isSorted)
                    continue;
                ++incompleteDeps;
            }
            if (incompleteDeps > bestIncompleteDeps)
                continue;
            if (bestGroup && bestGroup->sortedJobs.size() > group.sortedJobs.size())
                continue;
            if (bestGroup && bestGroup->inReadDeps.size() > group.inReadDeps.size())
                continue;
            bestGroup = &group;
            bestIncompleteDeps = incompleteDeps;
        }
        assert(bestGroup != nullptr); // Sort logic is broken if this is triggered

        bestGroup->isSorted = true;
        sortedGroups.push_back(bestGroup);
    }
}

template<typename T>
inline JobTree * JobTree::New () {
    static_assert(std::is_base_of<IUpdateGroup, T>::value, "JobTree only works on IUpdateGroups");

    // Stage 1: Gather job dependency data
    // Stage 2: Group by hard dependencies (write collision, circular read, before/after)
    // Stage 3: Build dependency graph for each group with run before/after as the deps
    // Stage 4: Topological sort groups, preferring nodes that have the most incoming reads first
    //          Note: Circular dependency in this stage is a user created failure and should assert
    // Stage 5: Build dependency graph of groups with reads as the dependencies
    // Stage 6: Topological sort dependency graph preferring largest group size, followed by incoming reads
    //          Note: Circular dependency in this stage should be allowed
    // Stage 7: Insert groups into the tree at their lowest depended on node

    std::vector<UpdateGroupJob> & updateGroupJobs = GetUpdateGroupJobs<T>();

    JobTree * tree = new JobTree();
    tree->nodeMemory = new JobNode[updateGroupJobs.size()];

    std::vector<JobDependencyData> depData;
    for (size_t i = 0; i < updateGroupJobs.size(); ++i) {
        depData.push_back(JobDependencyData{ tree->nodeMemory + i, updateGroupJobs[i].id });
        tree->nodeMemory[i].job = updateGroupJobs[i].factory();
    }

    // Stage 1: Gather job dependency data
    BuildJobDependencyData(depData);

    // Stage 2-4: Build hard dependency groups that will need to run in serial
    std::vector<DependencyGroup> hardGroups;
    BuildHardDependencyGroups(hardGroups, depData);

    // Stage 5: Build dependency graph between groups
    BuildGroupDependencyData(hardGroups);

    // Stage 6: Topological sort the groups
    std::vector<DependencyGroup*> sortedGroups;
    SortDependencyGroups(hardGroups, sortedGroups);

    // Stage 7: Insert groups into the tree at their lowest depended on node
    tree->InsertIntoTree(sortedGroups);

    return tree;
}

inline void JobTree::RunJobList (std::vector<impl::JobNode*> & list, std::vector<std::future<void>> & tasks, Timestep dt) {
    for (impl::JobNode * node : list) {
        tasks.push_back(std::async(std::launch::async, [node, dt]() {
            impl::JobNode * currentNode = node;
            // Run the assigned job
            currentNode->job->Run(dt);

            // Just continue down our dependents if we only have one
            while (currentNode->dependents.size() == 1) {
                currentNode = currentNode->dependents[0];
                currentNode->job->Run(dt);
            }

            // Schedule our dependents to be run
            RunJobList(currentNode->dependents, currentNode->runningTasks, dt);
        }));
    }

    for (auto & task : tasks)
        task.wait();
    tasks.clear();
}

inline void JobTree::Run (Timestep dt, bool singleThreaded) {
    if (singleThreaded) {
        ForEachNode([dt](impl::JobNode * node) {
            node->job->Run(dt);
        });
    }
    else {
        RunJobList(topNodes, runningTasks, dt);
    }

    ForEachNode([](impl::JobNode * node) {
        node->job->ApplyQueuedCommands();
    });
}

} // namespace impl
} // namespace ecs
