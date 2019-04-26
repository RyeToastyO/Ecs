/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

namespace ecs {

bool Entity::operator== (const Entity & rhs) const {
    return index == rhs.index && generation == rhs.generation;
}

} // namespace ecs
