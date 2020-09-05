// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include "helpers/hash.h"

namespace ecs {

// Must be included in any struct that is to be used as a component, including
// singletons. The name is used for hashing to a unique ID. If you run into the
// situation of hash collisions, changing just this name can fix the issue. The
// hash function is implemented manually so that it is consistent between
// executions and platforms.
#define ECS_COMPONENT(uniqueName)                                                       \
    static ::ecs::impl::ComponentId GetComponentId () {                                 \
        static ::ecs::impl::ComponentId s_id = ::ecs::impl::StringHash(#uniqueName);    \
        return s_id;                                                                    \
    }

// - Create a struct that inherits ecs::ISingletonComponent
// - Guaranteed to exist
// - One per Manager, use Manager->GetSingletonComponent<T>()
// - Safe to contain dynamically allocated data
struct ISingletonComponent {
    virtual ~ISingletonComponent () {}
};

namespace impl {

typedef uint64_t ComponentId;

template<typename T>
ComponentId GetComponentId ();

template<typename T>
size_t GetComponentSize ();

} // namespace impl
} // namespace ecs
