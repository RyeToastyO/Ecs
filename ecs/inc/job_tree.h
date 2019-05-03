/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "job.h"
#include "update_group.h"

#include <cstdint>
#include <future>
#include <vector>

namespace ecs {
namespace impl {

struct JobNode {
    Job * job = nullptr;
    std::vector<JobNode*> dependents;
    std::vector<std::future<void>> runningTasks;

    ~JobNode ();
};

struct DependencyGroup;
struct JobDependencyData;
struct LowestNode;

struct JobTree {
    JobNode * nodeMemory = nullptr;
    std::vector<JobNode*> topNodes;
    std::vector<std::future<void>> runningTasks;

    ~JobTree ();

    template<typename T>
    static JobTree * New ();

    void Run (Timestep dt);

    template<typename T>
    void ForEachNode (T func) const;

private:
    template<typename T>
    void ForEachNodeInternal (JobNode * node, T func) const;

private:
    static void AddToDependencyGroup (JobDependencyData * data, DependencyGroup * group);
    static void BuildHardDependencyGroups (std::vector<DependencyGroup> & groups, std::vector<JobDependencyData> & depData);
    static void BuildJobDependencyData (std::vector<JobDependencyData> & depData);
    static void FindLowestSatisfyingNode (JobNode * node, uint32_t depth, const ComponentFlags & flags, LowestNode & results);
    static void RunJobList (std::vector<impl::JobNode*> & list, std::vector<std::future<void>> & tasks, Timestep dt);
};

} // namespace impl
} // namespace ecs
