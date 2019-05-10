/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

namespace ecs {

template<typename T, typename...Args>
inline void Manager::AddComponents (Entity entity, T component, Args...args) {
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Do not add Entity as a component");

    if (!Exists(entity))
        return;
    auto & entityData = m_entityData[entity.index];
    auto composition = entityData.chunk->GetComposition();

    composition.SetComponents(component, args...);

    SetCompositionInternal(entityData, composition);
    SetComponentsInternal(entityData, component, args...);
}

template<typename T, typename...Args>
inline Entity Manager::CreateEntityImmediate (T component, Args...args) {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot be added to entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Do not add Entity as a component");

    // Compile the composition
    impl::Composition composition;
    composition.SetComponents(component, args...);

    // Create the entity
    Entity entity = CreateEntityImmediateInternal(composition);

    // Apply all the components to the chunk's memory
    SetComponentsInternal(m_entityData[entity.index], component, args...);

    return entity;
}

template<typename T>
inline T * Manager::GetSingletonComponent () {
    static_assert(std::is_base_of<ISingletonComponent, T>::value, "GetSingletonComponent<T> must inherit ISingletonComponent");
    static_assert(!std::is_empty<T>(), "Singleton components must have data, they always exist so they can't be used as tags");

    auto iter = m_singletonComponents.find(impl::GetComponentId<T>());
    if (iter == m_singletonComponents.end()) {
        T * component = new T();
        m_singletonComponents.emplace(impl::GetComponentId<T>(), component);
        return component;
    }
    return static_cast<T*>(iter->second);
}

template<typename T>
inline bool Manager::HasComponent (Entity entity) const {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot exist on entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Yes, it does. Use Exists to check for deletion");

    if (!Exists(entity))
        return false;
    const auto & entityData = m_entityData[entity.index];
    return entityData.chunk->GetComponentFlags().Has<T>();
}

template<typename T>
inline T * Manager::FindComponent (Entity entity) const {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot be exist on entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Why are you finding an Entity with that Entity?");

    if (!Exists(entity))
        return nullptr;
    const auto & entityData = m_entityData[entity.index];
    return entityData.chunk->Find<T>(entityData.chunkIndex);
}

template<typename T, typename...Args>
inline void Manager::RemoveComponents (Entity entity) {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot exist on entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Do not remove Entity as a component");

    if (!Exists(entity))
        return;
    auto & entityData = m_entityData[entity.index];
    auto composition = entityData.chunk->GetComposition();

    composition.RemoveComponents<T, Args...>();

    SetCompositionInternal(entityData, composition);
}

template<typename T, typename...Args>
inline void Manager::SetComponentsInternal (const impl::EntityData & entity, std::shared_ptr<T> component, Args...args) const {
    static_assert(std::is_base_of<ISharedComponent, T>::value, "Shared components must inherit ISharedComponent");
    ECS_REF(component);
    SetComponentsInternal(entity, args...);
}

template<typename T, typename...Args>
inline void Manager::SetComponentsInternal (const impl::EntityData & entity, T component, Args...args) const {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot be set on entities");
    static_assert(!std::is_base_of<ISharedComponent, T>::value, "Don't directly add shared components, must be a std::shared_ptr<SharedComponentType>");
    *(entity.chunk->Find<T>(entity.chunkIndex)) = component;
    SetComponentsInternal(entity, args...);
}

inline Manager::Manager () {
}

inline Manager::~Manager () {
    for (auto & chunk : m_chunks)
        delete chunk.second;
    for (auto & job : m_manualJobs)
        delete job.second;
    for (auto & singleton : m_singletonComponents)
        delete singleton.second;
    for (auto & updateGroup : m_updateGroups)
        delete updateGroup.second;
    m_chunks.clear();
    m_manualJobs.clear();
    m_updateGroups.clear();
    m_singletonComponents.clear();
}

inline bool Manager::Exists (Entity entity) const {
    if (!entity.generation || m_entityData.size() <= entity.index)
        return false;
    return m_entityData[entity.index].generation == entity.generation;
}

inline Entity Manager::CreateEntityImmediate () {
    auto emptyComposition = impl::Composition();
    return CreateEntityImmediateInternal(emptyComposition);
}

inline Entity Manager::CreateEntityImmediateInternal (impl::Composition & composition) {
    // All entities have their entity handle added as a component
    // This is so that jobs can have the entity array easily passed along
    // using the same APIs as components
    composition.SetComponents(Entity{});

    // Find or create a chunk with this composition
    auto chunk = GetOrCreateChunk(composition);

    // Recycle or create a new EntityData
    uint32_t index;
    if (m_freeList.size() > 0) {
        index = m_freeList[m_freeList.size() - 1];
        m_freeList.pop_back();
    }
    else {
        m_entityData.push_back(impl::EntityData());
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

inline void Manager::DestroyImmediate (Entity entity) {
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

inline impl::Chunk * Manager::GetOrCreateChunk (const impl::Composition & composition) {
    auto chunkIter = m_chunks.find(composition);
    if (chunkIter == m_chunks.end()) {
        m_chunks.emplace(composition, new impl::Chunk(composition));
        chunkIter = m_chunks.find(composition);
        NotifyChunkCreated(chunkIter->second);
    }
    return chunkIter->second;
}

inline void Manager::NotifyChunkCreated (impl::Chunk * chunk) {
    for (const auto & jobIter : m_manualJobs)
        jobIter.second->OnChunkAdded(chunk);
    for (const auto & groupIter : m_updateGroups) {
        groupIter.second->ForEachNode([chunk](impl::JobNode * node) {
            node->job->OnChunkAdded(chunk);
        });
    }
}

template<typename T>
inline void Manager::RunJob (Timestep dt) {
    static_assert(std::is_base_of<Job, T>::value, "Must inherit from Job");

    Job * job = nullptr;

    auto iter = m_manualJobs.find(impl::GetJobId<T>());
    if (iter == m_manualJobs.end()) {
        job = new T();

        RegisterJobInternal(job);

        m_manualJobs.emplace(impl::GetJobId<T>(), job);
    }
    else {
        job = iter->second;
    }

    job->Run(dt);
    job->ApplyQueuedCommands();
}

template<typename T>
inline void Manager::RunUpdateGroup (Timestep dt) {
    static_assert(std::is_base_of<IUpdateGroup, T>::value, "Must inherit from IUpdateGroup");

    auto iter = m_updateGroups.find(impl::GetUpdateGroupId<T>());
    if (iter == m_updateGroups.end()) {
        BuildJobTreeInternal<T>();
        iter = m_updateGroups.find(impl::GetUpdateGroupId<T>());
    }

    iter->second->Run(dt);
}

template<typename T>
inline void Manager::BuildJobTreeInternal () {
    impl::JobTree * tree = impl::JobTree::New<T>();

    tree->ForEachNode([this](impl::JobNode * node) {
        RegisterJobInternal(node->job);
    });

    m_updateGroups.emplace(impl::GetUpdateGroupId<T>(), tree);
}

inline void Manager::RegisterJobInternal (Job * job) {
    job->OnRegistered(this);
    for (auto & chunk : m_chunks)
        job->OnChunkAdded(chunk.second);
}

inline void Manager::SetCompositionInternal (impl::EntityData & entityData, const impl::Composition & composition) {
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
    Entity * swappedEntity = fromChunk->Find<Entity>(fromIndex);
    if (swappedEntity)
        m_entityData[swappedEntity->index].chunkIndex = fromIndex;
}

} // namespace ecs
