#include "pch.h"

#include "entity.h"

namespace ecs {

DEFINE_ECS_COMPONENT(Entity);

const Entity Entity::kInvalid = Entity{ 0, 0 };

bool Entity::operator== (const Entity & rhs) const {
    return index == rhs.index && generation == rhs.generation;
}

} // namespace ecs
