/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

namespace ecs {

// QueuedComponentCollection
template<typename T>
inline void QueuedComponentCollection<T>::Apply (Entity entity, uint32_t index, Manager * mgr) {
    mgr->AddComponents(entity, m_components[index]);
}

template<typename T>
inline void QueuedComponentCollection<T>::Clear () {
    m_components.clear();
}

template<typename T>
inline uint32_t QueuedComponentCollection<T>::Push (T && component) {
    m_components.push_back(component);
    return (uint32_t)m_components.size() - 1;
}

template<typename T>
inline void ComponentRemover<T>::Apply (Entity entity, Manager * mgr) {
    mgr->RemoveComponents<T>(entity);
}

// CommandQueue
inline CommandQueue::~CommandQueue () {
    for (auto & collection : m_queuedComponents)
        delete collection.second;
    for (auto & remover : m_componentRemovers)
        delete remover.second;
}

inline void CommandQueue::Apply (Manager * mgr) {
    Entity createdEntity;
    for (const auto & command : m_commands) {
        switch (command.type) {
            case ECommandType::AddComponent: {
                auto iter = m_queuedComponents.find(command.componentId);
                Entity target = command.entity.generation == 0 ? createdEntity : command.entity;
                iter->second->Apply(target, command.addComponentIndex, mgr);
            } break;
            case ECommandType::CreateEntity:
                createdEntity = mgr->CreateEntityImmediate();
                break;
            case ECommandType::DestroyEntity:
                mgr->DestroyImmediate(command.entity);
                break;
            case ECommandType::RemoveComponent: {
                auto iter = m_componentRemovers.find(command.componentId);
                iter->second->Apply(command.entity, mgr);
            } break;
        }
    }

    m_commands.clear();
    for (auto & collectionIter : m_queuedComponents)
        collectionIter.second->Clear();
}

template<typename T, typename...Args>
inline void CommandQueue::AddComponents (Entity entity, T component, Args...args) {
    auto iter = m_queuedComponents.find(GetComponentId<T>());
    if (iter == m_queuedComponents.end()) {
        m_queuedComponents.emplace(GetComponentId<T>(), new QueuedComponentCollection<T>());
        iter = m_queuedComponents.find(GetComponentId<T>());
    }

    QueuedComponentCollection<T> * collection = static_cast<QueuedComponentCollection<T>*>(iter->second);

    m_commands.push_back(Command{
        ECommandType::AddComponent,
        entity,
        GetComponentId<T>(),
        collection->Push(std::move(component))
    });

    AddComponents(entity, args...);
}

template<typename T, typename...Args>
inline void CommandQueue::CreateEntity (T component, Args...args) {
    m_commands.push_back(Command{
        ECommandType::CreateEntity,
        Entity{},
        0,  // ComponentId
        0   // ComponentIndex
    });

    AddComponents(Entity{}, component, args...);
}

inline void CommandQueue::DestroyEntity (Entity entity) {
    m_commands.push_back(Command{
        ECommandType::DestroyEntity,
        entity,
        0,  // ComponentId
        0   // ComponentIndex
    });
}

template<typename T, typename...Args>
inline void CommandQueue::RemoveComponents (Entity entity) {
    auto iter = m_componentRemovers.find(GetComponentId<T>());
    if (iter == m_componentRemovers.end())
        m_componentRemovers.emplace(GetComponentId<T>(), new ComponentRemover<T>());

    m_commands.push_back(Command{
        ECommandType::RemoveComponent,
        entity,
        GetComponentId<T>(),
        0   // ComponentIndex
    });

    RemoveComponents<Args...>(entity);
}

} // namespace ecs
