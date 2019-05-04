/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "../config.h"

#include <cstdint>
#include <unordered_map>

namespace ecs {

struct ISingletonComponent {
    virtual ~ISingletonComponent () {}
};

namespace impl {

typedef uint32_t ComponentId;

typedef void (*ComponentDestructor)(void * component);

struct ComponentRegistry {
    static std::unordered_map<ComponentId, ComponentDestructor> & GetComponentDestructorMap ();
    static std::unordered_map<ComponentId, size_t> & GetComponentSizeMap ();
    static ComponentId & GetComponentCounter ();
    static size_t GetComponentSize (ComponentId id);
    static ComponentId RegisterComponent (size_t size, ComponentDestructor destructor);
};

template<typename T>
static ComponentId GetComponentId ();

static size_t GetComponentSize (ComponentId id);
static void DestructComponent (ComponentId id, void * component);

} // namespace impl
} // namespace ecs
