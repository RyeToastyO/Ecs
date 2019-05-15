/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "component.h"

#include <cstdint>

namespace ecs {

typedef uint64_t EntityId;

// - A handle to an Entity
// - Safe to hold on to
// - Should only be used with the Manager that created it
struct Entity {
    uint32_t index = 0;
    uint32_t generation = 0;

    EntityId GetId () const;
    static Entity FromId (EntityId id);

    bool operator== (const Entity & rhs) const;
    bool operator!= (const Entity & rhs) const;
};

} // namespace ecs
