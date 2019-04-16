namespace ecs {

template<typename T, typename...Args>
void Manager::AddComponents (Entity entity, T component, Args...args) {
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
bool Manager::HasComponent (Entity entity) const {
    if (!Exists(entity))
        return false;
    const auto & entityData = m_entityData[entity.index];
    return entityData.chunk->GetComposition().Has<T>();
}

template<typename T>
T * Manager::FindComponent (Entity entity) const {
    if (!Exists(entity))
        return nullptr;
    const auto & entityData = m_entityData[entity.index];
    return entityData.chunk->Find<T>(entityData.chunkIndex);
}

template<typename T, typename...Args>
void Manager::RemoveComponents (Entity entity) {
    if (!Exists(entity))
        return;
    auto & entityData = m_entityData[entity.index];
    auto composition = entityData.chunk->GetComposition();

    composition.ClearFlags<T, Args...>();

    SetCompositionInternal(entityData, composition);
}

template<typename T, typename...Args>
void Manager::SetComponentsInternal (const EntityData & entity, T component, Args...args) const {
    *(entity.chunk->Find<T>(entity.chunkIndex)) = component;
    if constexpr (sizeof...(Args) > 0)
        SetComponentsInternal(entity, args...);
}

} // namespace ecs
