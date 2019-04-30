/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "chunk.h"
#include "entity.h"
#include "command_queue.h"
#include "../helpers/ref.h"

#include <cstdint>
#include <type_traits>
#include <vector>

namespace ecs {

class Manager;

namespace impl {
struct IComponentAccess;
typedef uint32_t JobId;
} // namespace impl

// Job base class
struct Job {
    const impl::ComponentFlags & GetReadFlags () const;
    const impl::ComponentFlags & GetWriteFlags () const;

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
    virtual void ForEach (Timestep dt) { ECS_REF(dt); }

private:
    bool IsValid (const impl::Chunk * chunk) const;

private:
    uint32_t m_currentIndex = 0;
    std::vector<impl::Chunk *> m_chunks;

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
    template<typename T, typename...Args> friend struct Exclude;
    void AddExclude (impl::IComponentAccess * access);
    template<typename T> friend struct Read;
    void AddRead (impl::IComponentAccess * access);
    template<typename T> friend struct ReadOther;
    void AddReadOther (impl::IComponentAccess * access);
    template<typename T> friend struct SingletonComponentAccess;
    template<typename T> friend struct ReadSingleton;
    void AddReadSingleton (impl::IComponentAccess * access);
    template<typename T, typename...Args> friend struct Require;
    void AddRequire (impl::IComponentAccess * access);
    template<typename T, typename...Args> friend struct RequireAny;
    void AddRequireAny (impl::IComponentAccess * access);
    template<typename T> friend struct Write;
    void AddWrite (impl::IComponentAccess * access);
    template<typename T> friend struct WriteOther;
    void AddWriteOther (impl::IComponentAccess * access);
    template<typename T> friend struct WriteSingleton;
    void AddWriteSingleton (impl::IComponentAccess * access);
};

} // namespace ecs
