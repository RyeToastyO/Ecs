#pragma once

#include "../config.h"

#include <cstdint>
#include <unordered_map>

namespace ecs {

typedef uint32_t ComponentId;

struct ISingletonComponent {
    virtual ~ISingletonComponent () {}
};

struct ComponentRegistry {
    static std::unordered_map<ComponentId, size_t> & GetComponentSizeMap ();
    static ComponentId & GetComponentCounter ();
    static size_t GetComponentSize (ComponentId id);
    static ComponentId RegisterComponent (size_t size);
};

template<typename T>
static ComponentId GetComponentId ();

static size_t GetComponentSize (ComponentId id);

} // namespace ecs

#include "inline/component.inl"
