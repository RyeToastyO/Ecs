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

struct JobTree {
    JobNode * nodeMemory = nullptr;
    std::vector<JobNode*> topNodes;

    ~JobTree ();
};

template<typename T>
void ForEachNode (JobTree * tree, T func);

JobTree * NewJobTree (const std::vector<JobFactory> & factories);

} // namespace impl
} // namespace ecs
