/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

namespace ecs {

JobNode::~JobNode () {
    if (job) {
        delete job;
        job = nullptr;
    }
}

struct JobDependencyData {
    Job * job = nullptr;
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

void FindLowestSatisfyingNode (JobNode * node, uint32_t depth, const ComponentFlags & flags, LowestNode & results) {
    if (results.depth < depth) {
        if (node->job->GetWriteFlags().HasAny(flags)) {
            results.node = node;
            results.depth = depth;
        }
    }
    for (auto & dependent : node->dependents)
        FindLowestSatisfyingNode(&dependent, depth + 1, flags, results);
}

void AddToDependencyGroup (JobDependencyData * data, DependencyGroup * group) {
    data->isGrouped = true;
    group->read.SetFlags(data->job->GetReadFlags());
    group->write.SetFlags(data->job->GetWriteFlags());
    group->jobs.push_back(data);
}

JobNode * NewJobTree (const std::vector<JobFactory> & factories) {
    JobNode * rootNode = new JobNode();

    std::vector<JobDependencyData> depData;
    for (auto factory : factories)
        depData.push_back(JobDependencyData{ factory() });

    // Gather dependency graph data
    for (auto i = 0; i < depData.size(); ++i) {
        auto & depI = depData[i];
        auto & readI = depI.job->GetReadFlags();
        auto & writeI = depI.job->GetWriteFlags();

        for (auto j = i + 1; j < depData.size(); ++j) {
            auto & depJ = depData[j];
            auto & readJ = depJ.job->GetReadFlags();
            auto & writeJ = depJ.job->GetWriteFlags();

            if (writeI.HasAny(writeJ) || writeI.HasAny(readJ) && writeJ.HasAny(readI)) {
                depI.hardDeps.push_back(depJ.job);
                depJ.hardDeps.push_back(depI.job);
            }
            else {
                if (writeI.HasAny(readJ)) {
                    depI.inReadDeps.push_back(depJ.job);
                    depJ.outReadDeps.push_back(depI.job);
                }
                if (writeJ.HasAny(readI)) {
                    depJ.inReadDeps.push_back(depI.job);
                    depI.outReadDeps.push_back(depJ.job);
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

            if (group.write.HasAny(depJ.job->GetWriteFlags())) {
                AddToDependencyGroup(&depJ, &group);

                // Reset our iterator since we might need to group other jobs that didn't previously
                j = i;
            }
        }

        // Sort jobs in the group by incoming reads
        std::sort(group.jobs.begin(), group.jobs.end(), [](const JobDependencyData * a, const JobDependencyData * b) {
            return a->inReadDeps.size() < b->inReadDeps.size();
        });
    }

    // Sort groups by longest chain
    std::sort(hardGroups.begin(), hardGroups.end(), [](const DependencyGroup & a, const DependencyGroup & b) {
        return a.jobs.size() < b.jobs.size();
    });

    // Insert groups into the tree at the lowest node that satisfies their combined read dependencies
    for (auto & group : hardGroups) {
        LowestNode results;
        results.depth = 0;
        results.node = rootNode;

        FindLowestSatisfyingNode(rootNode, 0, group.read, results);
        
        JobNode * currentNode = results.node;
        for (auto jobData : group.jobs) {
            currentNode->dependents.push_back(JobNode{ jobData->job });
            currentNode = &(currentNode->dependents.back());
        }
    }

    // TODO: Sort children by depth (this mostly happens by above sorts, but not 100%)

    return rootNode;
}

} // namespace ecs
