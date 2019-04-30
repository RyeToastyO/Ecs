/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include <vector>

namespace ecs {
namespace impl {

// UpdateGroupId Registration
struct UpdateGroupRegistry {
    static UpdateGroupId RegisterUpdateGroup ();
};

inline UpdateGroupId UpdateGroupRegistry::RegisterUpdateGroup () {
    static UpdateGroupId s_idCounter = 0;
    return s_idCounter++;
}

template<typename T>
struct UpdateGroupIdGetter {
    static UpdateGroupId GetId ();
};

template<typename T>
inline UpdateGroupId UpdateGroupIdGetter<T>::GetId () {
    static UpdateGroupId id = UpdateGroupRegistry::RegisterUpdateGroup();
    return id;
}

template<typename T>
inline UpdateGroupId GetUpdateGroupId () {
    static_assert(std::is_base_of<IUpdateGroup, T>::value, "Must inherit UpdateGroup to use GetUpdateGroupId");
    return UpdateGroupIdGetter<typename std::remove_const<T>::type>::GetId();
}

// Job Registration
template<typename T>
inline std::vector<JobFactory> & GetUpdateGroupJobs () {
    static std::vector<JobFactory> s_jobs;
    return s_jobs;
}

template<typename TJob, typename TUpdateGroup>
inline UpdateGroupId RegisterJobForUpdateGroup () {
    static_assert(std::is_base_of<Job, TJob>::value, "Must inherit Job");
    static_assert(std::is_base_of<IUpdateGroup, TUpdateGroup>::value, "Must inherit IUpdateGroup");

    auto & jobs = GetUpdateGroupJobs<TUpdateGroup>();
    jobs.push_back([]() { return static_cast<Job*>(new TJob()); });

    return GetUpdateGroupId<TUpdateGroup>();
}

} // namespace impl
} // namespace ecs
