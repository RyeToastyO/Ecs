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

struct IComponentAccess;
class Manager;

typedef uint32_t JobId;

// Job base class
struct Job {
    const ComponentFlags & GetReadFlags () const;
    const ComponentFlags & GetWriteFlags () const;

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
    bool IsValid (const Chunk * chunk) const;

private:
    uint32_t m_currentIndex = 0;
    std::vector<Chunk *> m_chunks;

    std::vector<IComponentAccess *> m_dataAccess;
    std::vector<IComponentAccess *> m_singletonAccess;

    std::vector<ComponentFlags> m_requireAny;
    ComponentFlags m_exclude;
    ComponentFlags m_required;

    ComponentFlags m_read;
    ComponentFlags m_write;

    CommandQueue m_commands;

    Manager * m_manager = nullptr;

private:
    friend class Manager;
    void ApplyQueuedCommands ();
    void OnChunkAdded (Chunk * chunk);
    void OnRegistered (Manager * manager);

private:
    template<typename T, typename...Args> friend struct Exclude;
    void AddExclude (IComponentAccess * access);
    template<typename T> friend struct Read;
    void AddRead (IComponentAccess * access);
    template<typename T> friend struct ReadOther;
    void AddReadOther (IComponentAccess * access);
    template<typename T> friend struct SingletonComponentAccess;
    template<typename T> friend struct ReadSingleton;
    void AddReadSingleton (IComponentAccess * access);
    template<typename T, typename...Args> friend struct Require;
    void AddRequire (IComponentAccess * access);
    template<typename T, typename...Args> friend struct RequireAny;
    void AddRequireAny (IComponentAccess * access);
    template<typename T> friend struct Write;
    void AddWrite (IComponentAccess * access);
    template<typename T> friend struct WriteOther;
    void AddWriteOther (IComponentAccess * access);
    template<typename T> friend struct WriteSingleton;
    void AddWriteSingleton (IComponentAccess * access);
};

} // namespace ecs
