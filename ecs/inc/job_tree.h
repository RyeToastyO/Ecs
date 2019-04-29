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

struct JobNode {
    Job * job = nullptr;
    std::vector<JobNode> dependents;

    ~JobNode ();
};

template<typename T>
void ForEachNode (JobNode * root, T func);

JobNode * NewJobTree (const std::vector<JobFactory> & factories);

} // namespace ecs
