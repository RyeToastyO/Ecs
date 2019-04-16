#pragma once

#include "chunk.h"
#include "entity.h"
#include "job.h"

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
    T * FindComponent (Entity entity) const;

    template<typename T, typename...Args>
    void RemoveComponents (Entity entity);

    void RegisterJob (Job * job);

    void Update (float dt);

private:
    ARRAY(EntityData) m_entityData;
    ARRAY(uint32_t) m_freeList;
    ARRAY(Job *) m_jobs;
    DICTIONARY(ComponentFlags, Chunk*) m_chunks;

private:
    Entity CreateEntityImmediateInternal (ComponentFlags composition);

    Chunk * GetOrCreateChunk (const ComponentFlags & composition);

    template<typename T, typename...Args>
    void SetComponentsInternal (const EntityData & entity, T component, Args...args) const;

    void SetCompositionInternal (EntityData & entityData, const ComponentFlags & composition);
};

} // namespace ecs

#include "manager.inl"
