/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "job.h"
#include "../helpers/token_combine.h"

#include <cstdint>
#include <type_traits>

namespace ecs {

struct IUpdateGroup {
    virtual ~IUpdateGroup () {}
};

#define ECS_REGISTER_JOB_FOR_UPDATE_GROUP(job, updateGroup)                                                             \
static_assert(std::is_base_of<::ecs::Job, job>::value, #job " does not inherit Job");                                   \
static_assert(std::is_base_of<::ecs::IUpdateGroup, updateGroup>::value, #updateGroup " does not inherit IUpdateGroup"); \
::ecs::impl::UpdateGroupId ECS_TOKEN_COMBINE(s_##job##updateGroup, __LINE__) = ::ecs::impl::RegisterJobForUpdateGroup<job, updateGroup>();

namespace impl {

typedef uint32_t UpdateGroupId;

// Registration
typedef Job* (*JobFactory)();

template<typename T>
UpdateGroupId GetUpdateGroupId ();

template<typename T>
std::vector<JobFactory> & GetUpdateGroupJobs ();

template<typename TJob, typename TUpdateGroup>
UpdateGroupId RegisterJobForUpdateGroup ();

} // namespace impl
} // namespace ecs
