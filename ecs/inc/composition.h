/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "component_flags.h"

#include <map>
#include <memory>

namespace ecs {
namespace impl {

struct Composition {
    const ComponentFlags & GetComponentFlags () const;

    size_t GetHash () const;
    bool operator== (const Composition & rhs) const;

    template<typename...Args>
    typename std::enable_if<(sizeof...(Args) == 0)>::type RemoveComponents () {}
    template<typename T, typename...Args>
    void RemoveComponents ();

    template<typename T, typename...Args>
    void SetComponents (T component, Args...args);

private:
    ComponentFlags m_flags;
    std::map<ComponentId, ISharedComponentPtr> m_shared;

private:
    template<typename...Args>
    typename std::enable_if<(sizeof...(Args) == 0)>::type RemoveSharedComponentsInternal () {}
    template<typename T, typename...Args>
    void RemoveSharedComponentsInternal ();

    void SetSharedComponentsInternal ();
    template<typename T, typename...Args>
    void SetSharedComponentsInternal (std::shared_ptr<T> component, Args...args);
    template<typename T, typename...Args>
    void SetSharedComponentsInternal (T component, Args...args);
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
