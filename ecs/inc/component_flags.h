// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include "component.h"

#include <cstdint>
#include <functional>
#include <unordered_set>

namespace ecs {
namespace impl {

using ComponentFlagIterator = std::unordered_set<ComponentId>::const_iterator;

struct ComponentFlags {
    ComponentFlags ();

    void Clear ();

    void ClearFlag (ComponentId id);
    template<typename...Args>
    typename std::enable_if<(sizeof...(Args) == 0)>::type ClearFlags () {};
    template<typename T, typename...Args>
    void ClearFlags ();
    void ClearFlags (const ComponentFlags& flags);

    void SetFlag (ComponentId id);
    template<typename...Args>
    typename std::enable_if<(sizeof...(Args) == 0)>::type SetFlags () {};
    template<typename T, typename...Args>
    void SetFlags ();
    void SetFlags (const ComponentFlags& flags);

    ComponentFlagIterator begin () const;
    ComponentFlagIterator end () const;

    bool HasAll (const ComponentFlags& rhs) const;
    bool HasAny (const ComponentFlags& rhs) const;
    bool HasNone (const ComponentFlags& rhs) const;

    bool Has (ComponentId id) const;
    template<typename T>
    bool Has () const { return Has(GetComponentId<T>()); }

    size_t GetHash () const;
    bool operator== (const ComponentFlags& rhs) const;

private:
    std::unordered_set<ComponentId> m_flags;
};

} // namespace impl
} // namespace ecs

namespace std {
    template <> struct hash<::ecs::impl::ComponentFlags> {
        size_t operator() (const ::ecs::impl::ComponentFlags& flags) const {
            return flags.GetHash();
        }
    };
};
