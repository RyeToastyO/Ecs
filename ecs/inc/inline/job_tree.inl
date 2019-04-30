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
inline void ForEachNode (JobNode * node, T func) {
    func(node);
    for (auto dep : node->dependents)
        ForEachNode(dep, func);
}

template<typename T>
inline void ForEachNode (JobTree * tree, T func) {
    for (auto node : tree->topNodes)
        ForEachNode(node, func);
}

inline void FindLowestSatisfyingNode (JobNode * node, uint32_t depth, const ComponentFlags & flags, LowestNode & results) {
    if (results.depth < depth) {
        if (node->job->GetWriteFlags().HasAny(flags)) {
            results.node = node;
            results.depth = depth;
        }
    }
    for (auto dependent : node->dependents)
        FindLowestSatisfyingNode(dependent, depth + 1, flags, results);
}

inline void AddToDependencyGroup (JobDependencyData * data, DependencyGroup * group) {
    data->isGrouped = true;
    group->read.SetFlags(data->node->job->GetReadFlags());
    group->write.SetFlags(data->node->job->GetWriteFlags());
    group->jobs.push_back(data);
}

inline JobTree * NewJobTree (const std::vector<JobFactory> & factories) {
    JobTree * tree = new JobTree();
    tree->nodeMemory = new JobNode[factories.size()];

    std::vector<JobDependencyData> depData;
    for (auto i = 0; i < factories.size(); ++i) {
        depData.push_back(JobDependencyData{ tree->nodeMemory + i });
        tree->nodeMemory[i].job = factories[i]();
    }

    // Gather dependency graph data
    for (auto i = 0; i < depData.size(); ++i) {
        auto & depI = depData[i];
        auto & readI = depI.node->job->GetReadFlags();
        auto & writeI = depI.node->job->GetWriteFlags();

        for (auto j = i + 1; j < depData.size(); ++j) {
            auto & depJ = depData[j];
            auto & readJ = depJ.node->job->GetReadFlags();
            auto & writeJ = depJ.node->job->GetWriteFlags();

            if (writeI.HasAny(writeJ) || writeI.HasAny(readJ) && writeJ.HasAny(readI)) {
                depI.hardDeps.push_back(depJ.node->job);
                depJ.hardDeps.push_back(depI.node->job);
            }
            else {
                if (writeI.HasAny(readJ)) {
                    depI.inReadDeps.push_back(depJ.node->job);
                    depJ.outReadDeps.push_back(depI.node->job);
                }
                if (writeJ.HasAny(readI)) {
                    depJ.inReadDeps.push_back(depI.node->job);
                    depI.outReadDeps.push_back(depJ.node->job);
                }
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

            if (group.write.HasAny(depJ.node->job->GetWriteFlags())) {
                AddToDependencyGroup(&depJ, &group);

                // Reset our iterator since we might need to group other jobs that didn't previously
                j = i;
            }
        }

        // Sort jobs in the group by incoming reads
        std::sort(group.jobs.begin(), group.jobs.end(), [](const JobDependencyData * a, const JobDependencyData * b) {
            return a->inReadDeps.size() < b->inReadDeps.size();
        });
        hardGroups.push_back(std::move(group));
    }

    // Sort groups by longest chain
    std::sort(hardGroups.begin(), hardGroups.end(), [](const DependencyGroup & a, const DependencyGroup & b) {
        return a.jobs.size() < b.jobs.size();
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
