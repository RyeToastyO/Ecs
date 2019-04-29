/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

namespace ecs {

template<typename T, typename...Args>
void Manager::AddComponents (Entity entity, T component, Args...args) {
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Do not add Entity as a component");

    if (!Exists(entity))
        return;
    auto & entityData = m_entityData[entity.index];
    auto composition = entityData.chunk->GetComposition();

    composition.SetFlags<T, Args...>();

    SetCompositionInternal(entityData, composition);
    SetComponentsInternal(entityData, component, args...);
}

template<typename T, typename...Args>
Entity Manager::CreateEntityImmediate (T component, Args...args) {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot be added to entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Do not add Entity as a component");

    // Compile the composition flags
    ComponentFlags composition;
    composition.SetFlags<T, Args...>();

    // Create the entity
    Entity entity = CreateEntityImmediateInternal(composition);

    // Apply all the components to the chunk's memory
    SetComponentsInternal(m_entityData[entity.index], component, args...);

    return entity;
}

template<typename T>
T * Manager::GetSingletonComponent () {
    static_assert(std::is_base_of<ISingletonComponent, T>::value, "GetSingletonComponent<T> must inherit ISingletonComponent");
    static_assert(!std::is_empty<T>(), "Singleton components must have data, they always exist so they can't be used as tags");

    auto iter = m_singletonComponents.find(GetComponentId<T>());
    if (iter == m_singletonComponents.end()) {
        T * component = new T();
        m_singletonComponents.emplace(GetComponentId<T>(), component);
        return component;
    }
    return static_cast<T*>(iter->second);
}

template<typename T>
bool Manager::HasComponent (Entity entity) const {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot exist on entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Yes, it does. Use Exists to check for deletion");

    if (!Exists(entity))
        return false;
    const auto & entityData = m_entityData[entity.index];
    return entityData.chunk->GetComposition().Has<T>();
}

template<typename T>
T * Manager::FindComponent (Entity entity) const {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot be exist on entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Why are you finding an Entity with that Entity?");

    if (!Exists(entity))
        return nullptr;
    const auto & entityData = m_entityData[entity.index];
    return entityData.chunk->Find<T>(entityData.chunkIndex);
}

template<typename T, typename...Args>
void Manager::RemoveComponents (Entity entity) {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot exist on entities");
    static_assert(!std::is_same<std::remove_const<T>::type, ::ecs::Entity>::value, "Do not remove Entity as a component");

    if (!Exists(entity))
        return;
    auto & entityData = m_entityData[entity.index];
    auto composition = entityData.chunk->GetComposition();

    composition.ClearFlags<T, Args...>();

    SetCompositionInternal(entityData, composition);
}

template<typename T, typename...Args>
void Manager::SetComponentsInternal (const EntityData & entity, T component, Args...args) const {
    static_assert(!std::is_base_of<ISingletonComponent, T>::value, "Singleton components cannot be set on entities");
    *(entity.chunk->Find<T>(entity.chunkIndex)) = component;
    if constexpr (sizeof...(Args) > 0)
        SetComponentsInternal(entity, args...);
}

Manager::Manager () {
}

Manager::~Manager () {
    for (auto & chunk : m_chunks)
        delete chunk.second;
    for (auto & job : m_manualJobs)
        delete job.second;
    for (auto & singleton : m_singletonComponents)
        delete singleton.second;
    m_chunks.clear();
    m_manualJobs.clear();
    m_updateGroups.clear();
    m_singletonComponents.clear();
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
        NotifyChunkCreated(chunkIter->second);
    }
    return chunkIter->second;
}

void Manager::NotifyChunkCreated (Chunk * chunk) {
    for (const auto & jobIter : m_manualJobs)
        jobIter.second->OnChunkAdded(chunk);
    for (const auto & groupIter : m_updateGroups) {
        for (const auto & listIter : groupIter.second) {
            for (auto job : listIter) {
                job->OnChunkAdded(chunk);
            }
        }
    }
}

template<typename T>
void Manager::RunJob (Timestep dt) {
    static_assert(std::is_base_of<Job, T>::value, "Must inherit from Job");

    Job * job = nullptr;

    auto iter = m_manualJobs.find(GetJobId<T>());
    if (iter == m_manualJobs.end()) {
        job = new T();

        RegisterJobInternal(job);

        m_manualJobs.emplace(GetJobId<T>(), job);
    }
    else {
        job = iter->second;
    }

    job->Run(dt);
    job->ApplyQueuedCommands();
}

template<typename T>
void Manager::RunUpdateGroup (Timestep dt) {
    static_assert(std::is_base_of<IUpdateGroup, T>::value, "Must inherit from IUpdateGroup");

    auto iter = m_updateGroups.find(GetUpdateGroupId<T>());
    if (iter == m_updateGroups.end()) {
        BuildJobTreeInternal(GetUpdateGroupId<T>(), GetUpdateGroupJobs<T>());
        iter = m_updateGroups.find(GetUpdateGroupId<T>());
    }

    RunJobTree(iter->second, dt);
}

void Manager::RunJobTree (JobNode * rootNode, Timestep dt) {
    // TODO: Run jobs
    // TODO: Apply queued commands
}

void Manager::BuildJobTreeInternal (UpdateGroupId id, std::vector<JobFactory> & factories) {
    // TODO: Push in reverse order for now, we should actually be doing
    // some sorting later, but this preserves the order well enough
    // for testing at the moment
    for (auto i = factories.size(); i > 0;) {
        auto job = factories[--i]();
        RegisterJobInternal(job);
        m_scratchJobArray.push_back(job);
    }

    ParallelJobLists lists;
    while (m_scratchJobArray.size()) {
        JobList list;
        list.push_back(m_scratchJobArray.back());
        m_scratchJobArray.pop_back();

        for (auto i = 0; i < m_scratchJobArray.size();) {
            Job * job = m_scratchJobArray[i];

            bool matchesAny = false;
            for (auto listJob : list) {
                matchesAny = true;
                if (listJob->m_read.HasAny(job->m_read))
                    break;
                if (listJob->m_read.HasAny(job->m_write))
                    break;
                if (listJob->m_write.HasAny(job->m_read))
                    break;
                if (listJob->m_write.HasAny(job->m_write))
                    break;
                matchesAny = false;
            }
            if (matchesAny) {
                list.push_back(job);
                m_scratchJobArray[i] = m_scratchJobArray.back();
                m_scratchJobArray.pop_back();
            }
            else {
                ++i;
            }
        }
        lists.push_back(std::move(list));
    }

    m_updateGroups.emplace(id, std::move(lists));
}

void Manager::RegisterJobInternal (Job * job) {
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

} // namespace ecs
