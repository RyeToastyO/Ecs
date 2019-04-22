#pragma once

#include <cstdint>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "chunk.h"
#include "entity.h"
#include "job.h"
#include "update_group.h"

namespace ecs {

struct EntityData {
    uint32_t generation = UINT32_MAX;
    uint32_t chunkIndex = 0;
    Chunk * chunk = nullptr;
};

class Manager {
public:
    Manager ();
    ~Manager ();

public:
    bool Exists (Entity entity) const;

    template<typename T, typename...Args>
    void AddComponents (Entity entity, T component, Args...args);

    Entity CreateEntityImmediate ();

    template<typename T, typename...Args>
    Entity CreateEntityImmediate (T component, Args...args);

    void DestroyImmediate (Entity entity);

    template<typename T>
    T * GetSingletonComponent ();

    template<typename T>
    bool HasComponent (Entity entity) const;

    template<typename T>
    T * FindComponent (Entity entity) const;

    template<typename T, typename...Args>
    void RemoveComponents (Entity entity);

    template<typename T>
    void RunJob (Timestep dt);

    template<typename T>
    void RunUpdateGroup (Timestep dt);

private:
    std::vector<EntityData> m_entityData;
    std::vector<uint32_t> m_freeList;
    std::unordered_map<JobId, Job*> m_manualJobs;
    std::unordered_map<UpdateGroupId, std::vector<Job*> > m_updateGroups;
    std::unordered_map<ComponentFlags, Chunk*> m_chunks;
    std::unordered_map<ComponentId, ISingletonComponent*> m_singletonComponents;

private:
    uint32_t m_jobIndex = 0;
    std::vector<Job*> * m_jobList = nullptr;
    std::mutex m_jobListLock;
    uint8_t m_readLocks[ECS_MAX_COMPONENTS] = {};
    ComponentFlags m_writeLocks;

private:
    bool AcquireLocksInternal (Job * job);
    void ReleaseLocksInternal (Job * job);

    Entity CreateEntityImmediateInternal (ComponentFlags composition);

    Chunk * GetOrCreateChunk (const ComponentFlags & composition);

    template<typename T, typename...Args>
    void SetComponentsInternal (const EntityData & entity, T component, Args...args) const;

    void SetCompositionInternal (EntityData & entityData, const ComponentFlags & composition);

    void RegisterJobInternal (Job * job);

    void RunJobListThreadedInternal (Timestep dt);
};

} // namespace ecs

#include "inline/manager.inl"
