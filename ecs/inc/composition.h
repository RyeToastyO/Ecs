// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include "component_flags.h"
#include "component_collection.h"

#include <unordered_map>

namespace ecs {
namespace impl {

typedef IComponentCollection* (*ComponentCollectionAllocateFunc)();
typedef std::unordered_map<ComponentId, ComponentCollectionAllocateFunc> ComponentCollectionFactory;

struct Composition {
    const ComponentFlags& GetComponentFlags () const;
    const ComponentCollectionFactory& GetComponentCollectionFactory () const;

    size_t GetHash () const;
    bool operator== (const Composition& rhs) const;

    void Clear ();

    template<typename...Args>
    typename std::enable_if<(sizeof...(Args) == 0)>::type RemoveComponents () {}
    template<typename T, typename...Args>
    void RemoveComponents ();

    template<typename T, typename...Args>
    void SetComponents (T component, Args...args);

private:
    ComponentFlags m_flags;
    ComponentCollectionFactory m_componentCollectionFactory;

private:
    void SetComponentsInternal ();
    template<typename T, typename...Args>
    void SetComponentsInternal (T component, Args...args);
};

} // namespace impl
} // namespace ecs

namespace std {
    template <> struct hash<::ecs::impl::Composition> {
        size_t operator() (const ::ecs::impl::Composition& composition) const {
            return composition.GetHash();
        }
    };
};
