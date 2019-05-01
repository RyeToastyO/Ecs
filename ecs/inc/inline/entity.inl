/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

namespace ecs {

inline bool Entity::operator== (const Entity & rhs) const {
    return index == rhs.index && generation == rhs.generation;
}

inline bool Entity::operator!= (const Entity & rhs) const {
    return !(*this == rhs);
}

} // namespace ecs
