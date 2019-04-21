#include <cstdint>
#include <type_traits>

#include "job.h"

namespace ecs {

struct IUpdateGroup {
    virtual ~IUpdateGroup () {}
};

#define ECS_REGISTER_JOB_FOR_UPDATE_GROUP(job, updateGroup)                                                         \
static_assert(std::is_base_of<Job, job>::value, #job " does not inherit Job");                                      \
static_assert(std::is_base_of<IUpdateGroup, updateGroup>::value, #updateGroup " does not inherit IUpdateGroup");    \
UpdateGroupId s_##job##updateGroup##__LINE__ = RegisterJobForUpdateGroup<job, updateGroup>();

}

#include "inline/update_group.inl"
