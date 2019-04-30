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

struct ComponentRegistry {
    static std::unordered_map<ComponentId, size_t> & GetComponentSizeMap ();
    static ComponentId & GetComponentCounter ();
    static size_t GetComponentSize (ComponentId id);
    static ComponentId RegisterComponent (size_t size);
};

template<typename T>
static ComponentId GetComponentId ();

static size_t GetComponentSize (ComponentId id);

} // namespace impl
} // namespace ecs
