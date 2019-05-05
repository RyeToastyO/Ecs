/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "component_flags.h"

#include <map>

namespace ecs {
namespace impl {

struct Composition {
    size_t GetHash () const;
    bool operator== (const Composition & rhs) const;

private:
    ComponentFlags m_flags;
    std::map<ComponentId, std::shared_ptr<ISharedComponent>> m_shared;
};

} // namespace impl
} // namespace ecs

namespace std {
    template <> struct hash<::ecs::impl::Composition> {
        size_t operator() (const ::ecs::impl::Composition & composition) const {
            return composition.GetHash();
        }
    };
};
