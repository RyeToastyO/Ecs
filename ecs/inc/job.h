#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

#include "chunk.h"
#include "entity.h"
#include "../helpers/ref.h"

namespace ecs {

struct IComponentAccess;
class Manager;

typedef uint32_t JobId;

// Job base class
struct Job {
    template<typename T>
    bool HasComponent (Entity entity) const;

public:
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

    Manager * m_manager = nullptr;

private:
    friend class Manager;
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

}

#include "inline/job.inl"
