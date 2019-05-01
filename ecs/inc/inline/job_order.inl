/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

namespace ecs {
namespace impl {

// Interface
struct IJobOrdering {
    virtual void ApplyTo (std::set<JobId> &) = 0;
};

// Helpers
template<typename...Args>
inline typename std::enable_if<(sizeof...(Args) == 0)>::type AddJobIds (std::set<JobId> &) {}

template<typename T, typename...Args>
inline void AddJobIds (std::set<JobId> & jobs) {
    jobs.emplace(GetJobId<T>());
    AddJobIds<Args...>(jobs);
}

// Implementations
template<typename T, typename...Args>
struct RunAfter : public IJobOrdering {
    inline RunAfter (Job & job) { job.AddRunAfter(this); }
    inline void ApplyTo (std::set<JobId> & jobs) override { AddJobIds<T, Args...>(jobs); }
};

template<typename T, typename...Args>
struct RunBefore : public IJobOrdering {
    inline RunBefore (Job & job) { job.AddRunBefore(this); }
    inline void ApplyTo (std::set<JobId> & jobs) override { AddJobIds<T, Args...>(jobs); }
};

} // namespace impl
} // namespace ecs
