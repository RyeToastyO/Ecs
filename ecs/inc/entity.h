#pragma once

#include <cstdint>

#include "component.h"

namespace ecs {

struct Entity {
    uint32_t index = 0;
    uint32_t generation = 0;

    bool operator== (const Entity & rhs) const;
};

}

#include "inline/entity.inl"
