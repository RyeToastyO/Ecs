/*
 * Copyright (c) 2020 Riley Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include <functional>

namespace ecs {
namespace impl {

inline void HashCombine (size_t & /* seed */) {}

template <typename T, typename... Rest>
inline void HashCombine (size_t & seed, const T & v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    HashCombine(seed, rest...);
}

} // namespace impl
} // namespace ecs
