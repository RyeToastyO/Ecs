/*
 * Copyright (c) 2020 Riley Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "chunk.h"
#include "entity.h"
#include "job.h"
#include "prefab.h"

#include <cstdint>
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

// - Used to:
//     - Create/Destroy/Modify Entities
//     - Lookup Entity components
//     - Get Singleton components
//     - Run Jobs
//     - Create and spawn Prefabs
// - Recommended usage:
//     - Create a single, global ecs::Manager (Multiple managers existing is supported)
//     - Call jobs from a single location so you can see the order in which they execute
// - Warnings:
//     - The Manager itself is not thread-safe
//         - Do not call non-const functions if there could be another thread also accessing the Manager
//         - Jobs must satisfy the following to be safely run at the same time:
//             - Do not read and write from the same components
//             - Do not create/destroy/change composition of entities
class Manager {
public:
    Manager ();
    ~Manager ();

public:
    bool Exists (Entity entity) const;

    template<typename T, typename...Args>
    void AddComponents (Entity entity, T component, Args...args);

    Entity Clone (Entity entity);

    Entity CreateEntityImmediate ();

    template<typename T, typename...Args>
    Entity CreateEntityImmediate (T component, Args...args);

    template<typename T, typename...Args>
    Prefab CreatePrefab (T component, Args...args);

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

    Entity SpawnPrefab (Prefab prefab);

private:
    std::vector<impl::EntityData> m_entityData;
    std::vector<uint32_t> m_freeList;
    std::unordered_map<impl::JobId, Job*> m_jobs;
    std::unordered_map<impl::Composition, impl::Chunk*> m_chunks;
    std::unordered_map<impl::ComponentId, ISingletonComponent*> m_singletonComponents;

private:
    uint32_t AllocateNewEntityInternal ();

    Entity CreateEntityImmediateInternal (impl::Composition & composition);

    impl::Chunk * GetOrCreateChunk (const impl::Composition & composition);

    void NotifyChunkCreated (impl::Chunk * chunk);

    void SetComponentsInternal (const impl::EntityData &) const {}
    template<typename T, typename...Args>
    void SetComponentsInternal (const impl::EntityData & entity, std::shared_ptr<T> component, Args...args) const;
    template<typename T, typename...Args>
    void SetComponentsInternal (const impl::EntityData & entity, T component, Args...args) const;

    void SetCompositionInternal (impl::EntityData & entityData, const impl::Composition & composition);

    void RegisterJobInternal (Job * job);
};

} // namespace ecs
