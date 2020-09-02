// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include "../config.h"

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace ecs {

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
    static std::vector<size_t>& GetComponentSizes ();
    static size_t GetComponentSize (ComponentId id);
    static ComponentId RegisterComponent (size_t size);
};

template<typename T>
static ComponentId GetComponentId ();

static size_t GetComponentSize (ComponentId id);

} // namespace impl
} // namespace ecs
