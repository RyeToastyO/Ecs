/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "chunk.h"
#include "entity.h"
#include "job.h"
#include "job_tree.h"
#include "update_group.h"

#include <cstdint>
#include <future>
#include <unordered_map>
#include <vector>

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
    std::unordered_map<UpdateGroupId, JobNode*> m_updateGroups;
    std::unordered_map<ComponentFlags, Chunk*> m_chunks;
    std::unordered_map<ComponentId, ISingletonComponent*> m_singletonComponents;
    std::vector<std::future<std::vector<JobNode*>*>> m_runningTasks;
    std::vector<Job*> m_scratchJobArray;

private:
    void BuildJobTreeInternal (UpdateGroupId id, std::vector<JobFactory> & factories);

    Entity CreateEntityImmediateInternal (ComponentFlags composition);

    Chunk * GetOrCreateChunk (const ComponentFlags & composition);

    void NotifyChunkCreated (Chunk * chunk);

    template<typename T, typename...Args>
    void SetComponentsInternal (const EntityData & entity, T component, Args...args) const;

    void SetCompositionInternal (EntityData & entityData, const ComponentFlags & composition);

    void RegisterJobInternal (Job * job);

    void RunJobTree (JobNode * rootNode, Timestep dt);
};

} // namespace ecs
