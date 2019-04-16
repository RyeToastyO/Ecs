#include "pch.h"

#include "manager.h"

namespace ecs {

Manager::Manager () {
    const auto & jobs = GetRegisteredJobs();
    for (auto job : jobs)
        RegisterJob(job);
}

Manager::~Manager () {
    for (auto & chunk : m_chunks)
        delete chunk.second;
    m_chunks.clear();
}

bool Manager::Exists (Entity entity) const {
    if (!entity.generation || m_entityData.size() <= entity.index)
        return false;
    return m_entityData[entity.index].generation == entity.generation;
}

Entity Manager::CreateEntityImmediate () {
    return CreateEntityImmediateInternal(ComponentFlags());
}

Entity Manager::CreateEntityImmediateInternal (ComponentFlags composition) {
    // All entities have their entity handle added as a component
    // This is so that jobs can have the entity array easily passed along
    // using the same APIs as components
    composition.SetFlags<Entity>();

    // Find or create a chunk with this composition
    auto chunk = GetOrCreateChunk(composition);

    // Recycle or create a new EntityData
    uint32_t index;
    if (m_freeList.size() > 0) {
        index = m_freeList[m_freeList.size() - 1];
        m_freeList.pop_back();
    }
    else {
        m_entityData.push_back(EntityData());
        index = (uint32_t)m_entityData.size() - 1;
    }

    // Assign the chunk data to the EntityData
    m_entityData[index].chunkIndex = chunk->AllocateEntity();
    m_entityData[index].chunk = chunk;

    // Create the entity handle
    Entity entity = Entity{ index, m_entityData[index].generation };

    // Set the entity component
    SetComponentsInternal(m_entityData[index], entity);

    return entity;
}

void Manager::DestroyImmediate (Entity entity) {
    if (!Exists(entity))
        return;
    const auto & data = m_entityData[entity.index];

    data.chunk->RemoveEntity(data.chunkIndex);

    // This code makes the assumption that removing an entity swaps the tail
    // entity with the removed entity in order to accomplish the removal
    Entity * swappedEntity = data.chunk->Find<Entity>(data.chunkIndex);
    if (swappedEntity)
        m_entityData[swappedEntity->index].chunkIndex = data.chunkIndex;

    if (--m_entityData[entity.index].generation)
        m_freeList.push_back(entity.index);
}

Chunk * Manager::GetOrCreateChunk (const ComponentFlags & composition) {
    auto chunkIter = m_chunks.find(composition);
    if (chunkIter == m_chunks.end()) {
        m_chunks.emplace(composition, new Chunk(composition));
        chunkIter = m_chunks.find(composition);
        for (auto job : m_jobs)
            job->OnChunkAdded(chunkIter->second);
    }
    return chunkIter->second;
}

void Manager::RegisterJob (Job * job) {
    m_jobs.push_back(job);
    job->OnRegistered(this);
    for (auto & chunk : m_chunks)
        job->OnChunkAdded(chunk.second);
}

void Manager::SetCompositionInternal (EntityData & entityData, const ComponentFlags & composition) {
    auto chunk = GetOrCreateChunk(composition);
    if (chunk == entityData.chunk)
        return;
    entityData.chunkIndex = entityData.chunk->MoveTo(entityData.chunkIndex, *chunk);
    entityData.chunk = chunk;
}

void Manager::Update (float dt) {
    for (uint32_t i = 0; i < m_jobs.size(); ++i)
        m_jobs[i]->Run(dt);
}

} // namespace ecs
