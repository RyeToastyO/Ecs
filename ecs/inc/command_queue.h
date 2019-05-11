/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "entity.h"
#include "component.h"

#include <cstdint>
#include <vector>

namespace ecs {

class Manager;

namespace impl {

struct IComponentRemover {
    virtual void Apply (Entity entity, Manager * mgr) = 0;

    virtual ~IComponentRemover () {}
};

template<typename T>
struct ComponentRemover : public IComponentRemover {
    void Apply (Entity entity, Manager * mgr) override;

    virtual ~ComponentRemover () {}
};

struct IQueuedComponentCollection {
    virtual void Apply (Entity entity, uint32_t index, Manager * mgr) = 0;
    virtual void Clear () = 0;

    virtual ~IQueuedComponentCollection () {}
};

template<typename T>
struct QueuedComponentCollection : public IQueuedComponentCollection {
    void Apply (Entity entity, uint32_t index, Manager * mgr) override;
    void Clear () override;
    uint32_t Push (T && component);

    virtual ~QueuedComponentCollection () {}

private:
    std::vector<T> m_components;
};

enum ECommandType : uint8_t {
    AddComponent,
    CloneEntity,
    CreateEntity,
    DestroyEntity,
    RemoveComponent,
};

struct Command {
    ECommandType type;
    Entity entity;
    ComponentId componentId;
    uint32_t addComponentIndex;
};

struct CommandQueue {
    ~CommandQueue ();

    void Apply (Manager * mgr);

    void AddComponents (Entity) {}
    template<typename T, typename...Args>
    void AddComponents (Entity entity, T component, Args...args);

    void CloneEntity (Entity entity);

    template<typename T, typename...Args>
    void CreateEntity (T component, Args...args);

    void DestroyEntity (Entity entity);

    template<typename...Args>
    typename std::enable_if<(sizeof...(Args) == 0)>::type RemoveComponents (Entity) {}
    template<typename T, typename...Args>
    void RemoveComponents (Entity entity);

private:
    friend class Manager;
    std::vector<Command> m_commands;
    std::unordered_map<ComponentId, IComponentRemover*> m_componentRemovers;
    std::unordered_map<ComponentId, IQueuedComponentCollection*> m_queuedComponents;
};

} // namespace impl
} // namespace ecs
