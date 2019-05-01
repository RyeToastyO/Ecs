/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "job.h"
#include "update_group.h"

#include <cstdint>
#include <vector>

namespace ecs {
namespace impl {

struct JobNode {
    Job * job = nullptr;
    std::vector<JobNode*> dependents;

    ~JobNode ();
};

struct DependencyGroup;
struct JobDependencyData;
struct LowestNode;

struct JobTree {
    JobNode * nodeMemory = nullptr;
    std::vector<JobNode*> topNodes;

    ~JobTree ();

    template<typename T>
    static JobTree * Create ();

private:
    static void AddToDependencyGroup (JobDependencyData * data, DependencyGroup * group);
    static void FindLowestSatisfyingNode (JobNode * node, uint32_t depth, const ComponentFlags & flags, LowestNode & results);
};

template<typename T>
void ForEachNode (JobTree * tree, T func);

} // namespace impl
} // namespace ecs
