/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "../config.h"

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace ecs {

// - Create a struct that inherits ecs::ISharedComponent
// - Make instances of that struct as std::shared_ptr<T> with the desired contents
// - Assign those std::shared_ptr<T> to Entitys
// - Groups entities with other entities that have the same shared components
//     - Use when you can get large benefits from batch operations in ForEachChunk
//       or to store large pieces of data, like raw model or sprite data you don't want to duplicate
struct ISharedComponent {
    virtual ~ISharedComponent () {}
};
typedef std::shared_ptr<ISharedComponent> ISharedComponentPtr;

// - Create a struct that inherits ecs::ISingletonComponent
// - Guaranteed to exist
// - One per Manager, use Manager->GetSingletonComponent<T>()
// - Safe to contain dynamically allocated data
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
