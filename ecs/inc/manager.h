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

namespace impl {

struct EntityData {
    uint32_t generation = UINT32_MAX;
    uint32_t chunkIndex = 0;
    Chunk * chunk = nullptr;
};

} // namespace impl

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
    std::vector<impl::EntityData> m_entityData;
    std::vector<uint32_t> m_freeList;
    std::unordered_map<impl::JobId, Job*> m_manualJobs;
    std::unordered_map<impl::UpdateGroupId, impl::JobTree*> m_updateGroups;
    std::unordered_map<impl::ComponentFlags, impl::Chunk*> m_chunks;
    std::unordered_map<impl::ComponentId, ISingletonComponent*> m_singletonComponents;
    std::vector<std::future<std::vector<impl::JobNode*>*>> m_runningTasks;

private:
    template<typename T>
    void BuildJobTreeInternal ();

    Entity CreateEntityImmediateInternal (impl::ComponentFlags composition);

    impl::Chunk * GetOrCreateChunk (const impl::ComponentFlags & composition);

    void NotifyChunkCreated (impl::Chunk * chunk);

    template<typename...Args>
    typename std::enable_if<(sizeof...(Args) == 0)>::type SetComponentsInternal (const impl::EntityData &, Args...) const {}
    template<typename T, typename...Args>
    void SetComponentsInternal (const impl::EntityData & entity, T component, Args...args) const;

    void SetCompositionInternal (impl::EntityData & entityData, const impl::ComponentFlags & composition);

    void RegisterJobInternal (Job * job);

    void RunJobList (std::vector<impl::JobNode*> & list, Timestep dt);
    void RunJobTree (impl::JobTree * tree, Timestep dt);
};

} // namespace ecs
