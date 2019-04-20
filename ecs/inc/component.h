#pragma once

#include <cstdint>
#include <unordered_map>

namespace ecs {

// This can be large during development.
// Once you actually know your shipping component count,
// this should be set to it to speed up ComponentFlags
#ifndef MAX_ECS_COMPONENTS
#define MAX_ECS_COMPONENTS 256
#endif

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
