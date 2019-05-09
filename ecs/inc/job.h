/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "chunk.h"
#include "entity.h"
#include "command_queue.h"
#include "component_access.h"
#include "job_order.h"
#include "helpers/ref.h"

#include <cstdint>
#include <unordered_set>
#include <type_traits>
#include <vector>

namespace ecs {

class Manager;

namespace impl {

typedef uint32_t JobId;

struct IComponentAccess;
template<typename T> struct SingletonComponentAccess;
template<typename T, typename...Args> struct Exclude;
template<typename T> struct Read;
template<typename T> struct ReadOther;
template<typename T> struct ReadSingleton;
template<typename T, typename...Args> struct Require;
template<typename T, typename...Args> struct RequireAny;
template<typename T> struct Write;
template<typename T> struct WriteOther;
template<typename T> struct WriteSingleton;

struct IJobOrdering;
template<typename T, typename...Args> struct RunAfter;
template<typename T, typename...Args> struct RunBefore;

struct JobTree;

} // namespace impl

// - Create a struct that inherits ecs::Job
// - Override one or more of the following (most commonly just ForEach):
//     - Run
//         - Used to do work before and after per-entity work
//         - Call Job::Run(dt) to allow the ForEachChunk and ForEach to run
//     - ForEachChunk
//         - Used to do batch work on contiguous chunks of entities
//         - Call Job::ForEachChunk(dt) to allow ForEach to run
//     - ForEach
//         - Used to do work on each entity
// - Specify your entity filters using the macros from component_access.h
// - Run manually using Manager->RunJob<JobType>(Timestep)
// - Register for an IUpdateGroup using ECS_REGISTER_JOB_FOR_UPDATE_GROUP(JobType, UpdateGroupType)
// - Specify explicit job ordering within a group using the macros from job_order.h
struct Job {
    uint32_t GetChunkEntityCount () const;

    template<typename T>
    bool HasComponent (Entity entity) const;

    template<typename T, typename...Args>
    void QueueAddComponents (Entity, T component, Args...args);

    template<typename T, typename...Args>
    void QueueCreateEntity (T component, Args...args);

    void QueueDestroyEntity (Entity entity);

    template<typename T, typename...Args>
    void QueueRemoveComponents (Entity entity);

public:
    virtual ~Job () {}

    virtual void Run (Timestep dt);
    virtual void ForEachChunk (Timestep dt);
    // - Override to do work on each entity that satisfies the job's accessors
    virtual void ForEach (Timestep dt) { ECS_REF(dt); }

private:
    bool IsValid (const impl::Chunk * chunk) const;

private:
    uint32_t m_chunkIndex = 0;
    uint32_t m_entityIndex = 0;
    std::vector<impl::Chunk *> m_chunks;

    std::unordered_set<impl::JobId> m_runAfter;
    std::unordered_set<impl::JobId> m_runBefore;

    std::vector<impl::IComponentAccess *> m_dataAccess;
    std::vector<impl::IComponentAccess *> m_singletonAccess;

    std::vector<impl::ComponentFlags> m_requireAny;
    impl::ComponentFlags m_exclude;
    impl::ComponentFlags m_required;

    impl::ComponentFlags m_read;
    impl::ComponentFlags m_write;

    impl::CommandQueue m_commands;

    Manager * m_manager = nullptr;

private:
    friend class Manager;
    void ApplyQueuedCommands ();
    void OnChunkAdded (impl::Chunk * chunk);
    void OnRegistered (Manager * manager);

private:
    template<typename T, typename...Args> friend struct impl::Exclude;
    void AddExclude (impl::IComponentAccess * access);
    template<typename T> friend struct impl::Read;
    void AddRead (impl::IComponentAccess * access);
    template<typename T> friend struct impl::ReadOther;
    void AddReadOther (impl::IComponentAccess * access);
    template<typename T> friend struct impl::SingletonComponentAccess;
    template<typename T> friend struct impl::ReadSingleton;
    void AddReadSingleton (impl::IComponentAccess * access);
    template<typename T, typename...Args> friend struct impl::Require;
    void AddRequire (impl::IComponentAccess * access);
    template<typename T, typename...Args> friend struct impl::RequireAny;
    void AddRequireAny (impl::IComponentAccess * access);
    template<typename T> friend struct impl::Write;
    void AddWrite (impl::IComponentAccess * access);
    template<typename T> friend struct impl::WriteOther;
    void AddWriteOther (impl::IComponentAccess * access);
    template<typename T> friend struct impl::WriteSingleton;
    void AddWriteSingleton (impl::IComponentAccess * access);

private:
    template<typename T, typename...Args> friend struct impl::RunAfter;
    void AddRunAfter (impl::IJobOrdering * ordering);
    template<typename T, typename...Args> friend struct impl::RunBefore;
    void AddRunBefore (impl::IJobOrdering * ordering);

private:
    friend struct impl::JobTree;
    const std::unordered_set<impl::JobId> & GetRunAfter () const;
    const std::unordered_set<impl::JobId> & GetRunBefore () const;
    const impl::ComponentFlags & GetReadFlags () const;
    const impl::ComponentFlags & GetWriteFlags () const;
};

namespace impl {

// Registration
template<typename T> JobId GetJobId ();

} // namespace impl

} // namespace ecs
