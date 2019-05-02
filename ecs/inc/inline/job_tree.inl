/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

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
    std::vector<Job*> hardDeps; // Writes and RUN_BEFORE/AFTER
    std::vector<Job*> outReadDeps;
    std::vector<Job*> inReadDeps;
};

struct DependencyGroup {
    ComponentFlags read;
    ComponentFlags write;
    std::vector<JobDependencyData*> jobs;
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
    std::vector<UpdateGroupJob> & updateGroupJobs = GetUpdateGroupJobs<T>();

    JobTree * tree = new JobTree();
    tree->nodeMemory = new JobNode[updateGroupJobs.size()];

    std::vector<JobDependencyData> depData;
    for (auto i = 0; i < updateGroupJobs.size(); ++i) {
        depData.push_back(JobDependencyData{ tree->nodeMemory + i, updateGroupJobs[i].id });
        tree->nodeMemory[i].job = updateGroupJobs[i].factory();
    }

    // Gather dependency graph data
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

            bool after = runAfterI.find(depJ.id) != runAfterI.end() || runAfterJ.find(depI.id) != runAfterJ.end();
            bool before = runBeforeI.find(depJ.id) != runBeforeI.end() || runBeforeJ.find(depI.id) != runBeforeJ.end();

            bool writeMatch = writeI.HasAny(writeJ);
            bool circularRead = writeI.HasAny(readJ) && writeJ.HasAny(readI);
            bool explicitOrder = before || after;
            if (writeMatch || circularRead || explicitOrder) {
                depI.hardDeps.push_back(depJ.node->job);
                depJ.hardDeps.push_back(depI.node->job);
            }
            else if (writeI.HasAny(readJ)) {
                depI.inReadDeps.push_back(depJ.node->job);
                depJ.outReadDeps.push_back(depI.node->job);
            }
            else if (writeJ.HasAny(readI)) {
                depJ.inReadDeps.push_back(depI.node->job);
                depI.outReadDeps.push_back(depJ.node->job);
            }
        }
    }

    // Build hard dependency groups that will need to run in serial
    std::vector<DependencyGroup> hardGroups;
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

            // TODO: Take into account run before/after
            if (group.write.HasAny(depJ.node->job->GetWriteFlags())) {
                AddToDependencyGroup(&depJ, &group);

                // Reset our iterator since we might need to group other jobs that didn't previously
                j = i;
            }
        }

        // Sort jobs in the group by incoming reads
        std::sort(group.jobs.begin(), group.jobs.end(), [](const JobDependencyData * a, const JobDependencyData * b) {
            return a->inReadDeps.size() > b->inReadDeps.size();
        });

        // A (Before B)
        // D (After A)
        // D, E, B, F, C, A, G
        // D, E, A, B, F, C, G
        // E, A, D, B, F, C, G

        // Selection sort by explicit order
        // TODO: validate that we don't have any cycles, currently causes infinite loop
        for (auto mainIndex = 0; mainIndex < group.jobs.size(); ++mainIndex) {
            for (auto secondIndex = mainIndex + 1; secondIndex < group.jobs.size(); ++secondIndex) {
                auto mainId = group.jobs[mainIndex]->id;
                auto secondId = group.jobs[secondIndex]->id;
                auto & mainAfter = group.jobs[mainIndex]->node->job->GetRunAfter();
                auto & secondBefore = group.jobs[secondIndex]->node->job->GetRunBefore();

                if (mainAfter.find(secondId) != mainAfter.end() || secondBefore.find(mainId) != secondBefore.end()) {
                    auto secondJobDepData = group.jobs[secondIndex];
                    group.jobs.erase(group.jobs.begin() + secondIndex);
                    group.jobs.insert(group.jobs.begin() + mainIndex, secondJobDepData);

                    // This is required for correctness, but can create infinite loops
                    secondIndex = mainIndex;
                }
            }
        }
        for (auto mainIndex = (int32_t)group.jobs.size() - 1; mainIndex >= 0; --mainIndex) {
            for (auto secondIndex = mainIndex - 1; secondIndex >= 0; --secondIndex) {
                auto mainId = group.jobs[mainIndex]->id;
                auto secondId = group.jobs[secondIndex]->id;
                auto & mainBefore = group.jobs[mainIndex]->node->job->GetRunBefore();
                auto & secondAfter = group.jobs[secondIndex]->node->job->GetRunAfter();

                if (mainBefore.find(secondId) != mainBefore.end() || secondAfter.find(mainId) != secondAfter.end()) {
                    auto secondJobDepData = group.jobs[secondIndex];
                    group.jobs.erase(group.jobs.begin() + secondIndex);
                    group.jobs.insert(group.jobs.begin() + mainIndex, secondJobDepData);

                    // This is required for correctness, but can create infinite loops
                    secondIndex = mainIndex;
                }
            }
        }

        hardGroups.push_back(std::move(group));
    }

    // Sort groups by longest chain
    std::sort(hardGroups.begin(), hardGroups.end(), [](const DependencyGroup & a, const DependencyGroup & b) {
        return a.jobs.size() > b.jobs.size();
    });

    // Insert groups into the tree at the lowest node that satisfies their combined read dependencies
    // TODO: there is a bug in this
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
