/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "../config.h"
#include "component.h"

#include <cstdint>
#include <functional>

namespace ecs {
namespace impl {

typedef uint64_t ComponentFlagDataType;
const auto COMPONENT_FLAG_DATA_BITS = sizeof(ComponentFlagDataType) * 8;
const auto COMPONENT_FLAG_DATA_COUNT = ((ECS_MAX_COMPONENTS - 1 + COMPONENT_FLAG_DATA_BITS) / COMPONENT_FLAG_DATA_BITS);

struct ComponentFlags;

struct ComponentFlagIterator {
    ComponentFlagIterator (const ComponentFlags & flags);
    ComponentFlagIterator (const ComponentFlags & flags, ComponentId id);

    bool operator!= (const ComponentFlagIterator & rhs) const;
    ComponentFlagIterator & operator++ ();
    const ComponentId & operator* () const;

    ComponentFlagIterator begin () const;
    ComponentFlagIterator end () const;
private:
    ComponentId m_current;
    const ComponentFlags & m_flags;
};

struct ComponentInfo {
    size_t ComponentCount = 0;
    size_t DataComponentCount = 0;
    size_t TotalSize = 0;
};

struct ComponentFlags {
    ComponentFlags ();

    void Clear ();

    void ClearFlag (ComponentId id);
    template<typename...Args>
    typename std::enable_if<(sizeof...(Args) == 0)>::type ClearFlags () {};
    template<typename T, typename...Args>
    void ClearFlags ();
    void ClearFlags (const ComponentFlags & flags);

    void SetFlag (ComponentId id);
    template<typename...Args>
    typename std::enable_if<(sizeof...(Args) == 0)>::type SetFlags () {};
    template<typename T, typename...Args>
    void SetFlags ();
    void SetFlags (const ComponentFlags & flags);

    ComponentInfo GetComponentInfo () const;
    ComponentFlagIterator GetIterator () const;

    bool HasAll (const ComponentFlags & rhs) const;
    bool HasAny (const ComponentFlags & rhs) const;
    bool HasNone (const ComponentFlags & rhs) const;

    bool Has (ComponentId id) const;
    template<typename T>
    bool Has () const { return Has(GetComponentId<T>()); }

    size_t GetHash () const;
    bool operator== (const ComponentFlags & rhs) const;

private:
    ComponentFlagDataType flags[COMPONENT_FLAG_DATA_COUNT];
};

} // namespace impl
} // namespace ecs

namespace std {
    template <> struct hash<::ecs::impl::ComponentFlags> {
        size_t operator() (const ::ecs::impl::ComponentFlags & flags) const {
            return flags.GetHash();
        }
    };
};
