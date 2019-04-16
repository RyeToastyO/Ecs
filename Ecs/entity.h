#pragma once

#include "component.h"

namespace ecs {

struct Entity {
    ECS_COMPONENT();

    uint32_t index = 0;
    uint32_t generation = 0;

    bool operator== (const Entity & rhs) const;

    static const Entity kInvalid;
};

}
