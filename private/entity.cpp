#include "../public/entity.h"

namespace ecs {

const Entity Entity::kInvalid = Entity{ 0, 0 };

bool Entity::operator== (const Entity & rhs) const {
    return index == rhs.index && generation == rhs.generation;
}

} // namespace ecs