// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

namespace ecs {

// - Adds components to an entity
// - Sets value if the components already exist
template<typename T, typename...Args>
inline void Manager::AddComponents (Entity entity, T component, Args...args) {
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Do not add Entity as a component");

    if (!Exists(entity))
        return;

    std::unique_lock<std::shared_mutex> lock(m_entityMutex);

    auto& entityData = m_entityData[entity.index];
    m_scratchComposition = entityData.chunk->GetComposition();

    m_scratchComposition.SetComponents(component, args...);

    SetCompositionInternal(entityData, m_scratchComposition);
    SetComponentsInternal(entityData, component, args...);
}


// - Creates an entity and adds the specified components to it
template<typename T, typename...Args>
inline Entity Manager::CreateEntityImmediate (T component, Args...args) {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot be added to entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Do not add Entity as a component");

    std::unique_lock<std::shared_mutex> lock(m_entityMutex);

    // Compile the composition
    m_scratchComposition.Clear();
    m_scratchComposition.SetComponents(component, args...);

    // Create the entity
    Entity entity = CreateEntityImmediateInternal(m_scratchComposition);

    // Apply all the components to the chunk's memory
    SetComponentsInternal(m_entityData[entity.index], component, args...);

    return entity;
}


// - Gets an ISingletonComponent
// - Guaranteed to exist
// - One per Manager
// - Pointer is safe for the Manager's lifetime
template<typename T>
inline T* Manager::GetSingletonComponent () {
    static_assert(std::is_base_of<ISingletonComponent, T>::value, "GetSingletonComponent<T> must inherit ISingletonComponent");
    static_assert(!std::is_empty<T>(), "Singleton components must have data, they always exist so they can't be used as tags");

    T* singleton = nullptr;
    {
        std::shared_lock<std::shared_mutex> lock(m_singletonMutex);

        auto iter = m_singletonComponents.find(impl::GetComponentId<T>());
        singleton = iter != m_singletonComponents.end() ? static_cast<T*>(iter->second) : nullptr;
    }

    if (!singleton) {
        std::unique_lock<std::shared_mutex> lock(m_singletonMutex);

        // We have to check again, in case this component was requested again after we released the shared lock.
        auto iter = m_singletonComponents.find(impl::GetComponentId<T>());
        singleton = iter != m_singletonComponents.end() ? static_cast<T*>(iter->second) : nullptr;

        if (!singleton) {
            singleton = new T();
            m_singletonComponents.emplace(impl::GetComponentId<T>(), singleton);
        }
    }
    return singleton;
}


// - Checks for existance of a component on an entity
template<typename T>
inline bool Manager::HasComponent (Entity entity) {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot exist on entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Yes, it does. Use Exists to check for deletion");

    if (!Exists(entity))
        return false;

    std::shared_lock<std::shared_mutex> lock(m_entityMutex);

    const auto& entityData = m_entityData[entity.index];
    return entityData.chunk->GetComponentFlags().Has<T>();
}


// - Gets a pointer to a component for an entity
// - nullptr if the entity doesn't have one, or the entity is destroyed
// - Pointer is not safe to hold on to and should be considered invalidated by:
//     - Composition changes to this entity or one with the same composition
//     - Destruction of this entity or one with the same composition
template<typename T>
inline T* Manager::FindComponent (Entity entity) {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot be exist on entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Why are you finding an Entity with that Entity?");
    static_assert(!std::is_empty<T>(), "Use HasComponent for tag components");

    if (!Exists(entity))
        return nullptr;

    std::shared_lock<std::shared_mutex> lock(m_entityMutex);

    const auto& entityData = m_entityData[entity.index];
    return entityData.chunk->Find<T>(entityData.chunkIndex);
}


// - Removes components from an entity if it has them
template<typename T, typename...Args>
inline void Manager::RemoveComponents (Entity entity) {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot exist on entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Do not remove Entity as a component");

    if (!Exists(entity))
        return;

    std::unique_lock<std::shared_mutex> lock(m_entityMutex);

    auto& entityData = m_entityData[entity.index];
    m_scratchComposition = entityData.chunk->GetComposition();

    m_scratchComposition.RemoveComponents<T, Args...>();

    SetCompositionInternal(entityData, m_scratchComposition);
}

template<typename T, typename...Args>
inline typename std::enable_if<std::is_empty<T>::value == 0>::type Manager::SetComponentsInternal (const impl::EntityData& entity, T component, Args...args) const {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot be set on entities");
    *(entity.chunk->Find<T>(entity.chunkIndex)) = component;
    SetComponentsInternal(entity, args...);
}

template<typename T, typename...Args>
inline typename std::enable_if<std::is_empty<T>::value>::type Manager::SetComponentsInternal (const impl::EntityData& entity, T component, Args...args) const {
    ECS_REF(component);
    SetComponentsInternal(entity, args...);
}

inline Manager::Manager () {
}

inline Manager::~Manager () {
    for (auto& chunk : m_chunks)
        delete chunk.second;
    for (auto& job : m_jobs)
        delete job.second;
    for (auto& singleton : m_singletonComponents)
        delete singleton.second;
    m_chunks.clear();
    m_jobs.clear();
    m_singletonComponents.clear();
}


// - Checks if a created entity has been destroyed
inline bool Manager::Exists (Entity entity) {
    std::shared_lock<std::shared_mutex> lock(m_entityMutex);

    if (!entity.generation || m_entityData.size() <= entity.index)
        return false;
    return m_entityData[entity.index].generation == entity.generation;
}


// - Creates a copy of an Entity
// - Returns an invalid Entity if the passed in entity has been destroyed
inline Entity Manager::Clone (Entity entity) {
    if (!Exists(entity))
        return Entity();

    std::unique_lock<std::shared_mutex> lock(m_entityMutex);

    // Use the chunk to actually copy component data
    impl::EntityData& entityData = m_entityData[entity.index];
    uint32_t chunkIndex = entityData.chunk->CloneEntity(entityData.chunkIndex);

    // Allocate a new entity for the entity we just cloned on the chunk
    uint32_t entityIndex = AllocateNewEntityInternal();

    // Point the entity data at the chunk
    m_entityData[entityIndex].chunk = m_entityData[entity.index].chunk; // Don't use entityData.chunk, it might be invalidated
    m_entityData[entityIndex].chunkIndex = chunkIndex;

    // Return the entity handle to the new entity
    Entity newEntity = Entity{ entityIndex, m_entityData[entityIndex].generation };

    // Set the entity component before we do, since it was just cloned by the chunk
    SetComponentsInternal(m_entityData[entityIndex], newEntity);

    return newEntity;
}


// - Creates an empty entity
// - Prefer initializing with components as it is more efficient than adding after creation
inline Entity Manager::CreateEntityImmediate () {
    std::unique_lock<std::shared_mutex> lock(m_entityMutex);

    m_scratchComposition.Clear();
    return CreateEntityImmediateInternal(m_scratchComposition);
}

inline Entity Manager::CreateEntityImmediateInternal (impl::Composition& composition) {
    // All entities have their entity handle added as a component
    // This is so that jobs can have the entity array easily passed along
    // using the same APIs as components
    composition.SetComponents(Entity{});

    // Find or create a chunk with this composition
    auto chunk = GetOrCreateChunk(composition);

    // Recycle or create a new EntityData
    uint32_t index = AllocateNewEntityInternal();

    // Assign the chunk data to the EntityData
    m_entityData[index].chunkIndex = chunk->AllocateEntity();
    m_entityData[index].chunk = chunk;

    // Create the entity handle
    Entity entity = Entity{ index, m_entityData[index].generation };

    // Set the entity component
    SetComponentsInternal(m_entityData[index], entity);

    return entity;
}

// - Creates a composition with default values
// - Create an entity from this prefab by calling SpawnPrefab(Prefab)
template<typename T, typename...Args>
inline Prefab Manager::CreatePrefab (T component, Args...args) {
    return Prefab{ CreateEntityImmediate(impl::PrefabComponent{}, component, args...) };
}

// - Causes Exists(entity) to return false
// - Removes an entity's component data from its chunk
// - Safe to call on an already destroyed entity
inline void Manager::DestroyImmediate (Entity entity) {
    if (!Exists(entity))
        return;

    std::unique_lock<std::shared_mutex> lock(m_entityMutex);

    const auto& data = m_entityData[entity.index];

    data.chunk->RemoveEntity(data.chunkIndex);

    // This code makes the assumption that removing an entity swaps the tail
    // entity with the removed entity in order to accomplish the removal
    Entity* swappedEntity = data.chunk->Find<Entity>(data.chunkIndex);
    if (swappedEntity)
        m_entityData[swappedEntity->index].chunkIndex = data.chunkIndex;

    if (--m_entityData[entity.index].generation)
        m_freeList.push_back(entity.index);
}


inline uint32_t Manager::AllocateNewEntityInternal () {
    // Recycle or create a new EntityData
    uint32_t index;
    if (m_freeList.size() > 0) {
        index = m_freeList.back();
        m_freeList.pop_back();
    }
    else {
        index = (uint32_t)m_entityData.size();
        m_entityData.push_back(impl::EntityData());
    }
    return index;
}

inline impl::Chunk* Manager::GetOrCreateChunk (const impl::Composition& composition) {
    auto chunkIter = m_chunks.find(composition);
    if (chunkIter == m_chunks.end()) {
        m_chunks.emplace(composition, new impl::Chunk(composition));
        chunkIter = m_chunks.find(composition);
        NotifyChunkCreated(chunkIter->second);
    }
    return chunkIter->second;
}

inline void Manager::NotifyChunkCreated (impl::Chunk* chunk) {
    std::unique_lock<std::shared_mutex> lock(m_jobMutex);

    for (const auto& jobIter : m_jobs)
        jobIter.second->OnChunkAdded(chunk);
}


// - Executes a job
// - Flushes any queued composition changes after running
template<typename T>
inline void Manager::RunJob () {
    static_assert(std::is_base_of<Job, T>::value, "Must inherit from Job");

    Job* job = nullptr;
    {
        std::shared_lock<std::shared_mutex> lock(m_jobMutex);

        auto iter = m_jobs.find(impl::GetJobId<T>());
        job = iter != m_jobs.end() ? iter->second : nullptr;
    }

    if (!job) {
        std::unique_lock<std::shared_mutex> lock(m_jobMutex);

        // Make sure that the job wasn't created when we released this lock for a few lines
        auto iter = m_jobs.find(impl::GetJobId<T>());
        job = iter != m_jobs.end() ? iter->second : nullptr;

        if (!job) {
            job = new T();

            RegisterJobInternal(job);

            m_jobs.emplace(impl::GetJobId<T>(), job);
        }
    }

    bool hasQueuedCommands = false;
    {
        // Don't allow entities changes or queued commands to run while we are running
        std::shared_lock<std::shared_mutex> entityLock(m_entityMutex);
        std::shared_lock<std::shared_mutex> queuedCommandLock(m_queuedCommandMutex);
        job->Run();
        hasQueuedCommands = job->HasQueuedCommands();
    }
    if (hasQueuedCommands) {
        // Don't allow other jobs to run while we are applying queued commands
        std::unique_lock<std::shared_mutex> lock(m_queuedCommandMutex);
        job->ApplyQueuedCommands();
    }
}


// - Creates an entity from a prefab
// - Will have all the components and values specified in the prefab
// - Passing an invalid Prefab will return an invalid Entity
inline Entity Manager::SpawnPrefab (Prefab prefab) {
    if (!Exists(prefab.m_entity))
        return Entity();

    Entity spawned = Clone(prefab.m_entity);
    RemoveComponents<impl::PrefabComponent>(spawned);

    return spawned;
}

inline void Manager::RegisterJobInternal (Job* job) {
    job->OnRegistered(this);
    for (auto& chunk : m_chunks)
        job->OnChunkAdded(chunk.second);
}

inline void Manager::SetCompositionInternal (impl::EntityData& entityData, const impl::Composition& composition) {
    auto chunk = GetOrCreateChunk(composition);
    if (chunk == entityData.chunk)
        return;

    auto fromChunk = entityData.chunk;
    auto fromIndex = entityData.chunkIndex;

    entityData.chunkIndex = entityData.chunk->MoveTo(entityData.chunkIndex, *chunk);
    entityData.chunk = chunk;

    // This code makes the assumption that removing an entity swaps the tail
    // entity with the removed entity in order to accomplish the removal
    // TODO: Find a way to make this systemic and the same code path as Delete
    Entity* swappedEntity = fromChunk->Find<Entity>(fromIndex);
    if (swappedEntity)
        m_entityData[swappedEntity->index].chunkIndex = fromIndex;
}

} // namespace ecs
