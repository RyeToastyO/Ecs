#pragma once

#include <cstdint>
#include <functional>

#include "component.h"

namespace ecs {

typedef uint64_t ComponentFlagDataType;
const auto COMPONENT_FLAG_DATA_BITS = sizeof(ComponentFlagDataType) * 8;
const auto COMPONENT_FLAG_DATA_COUNT = ((MAX_ECS_COMPONENTS - 1 + COMPONENT_FLAG_DATA_BITS) / COMPONENT_FLAG_DATA_BITS);

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

    template<typename T, typename...Args>
    void ClearFlags () {
        const auto id = GetComponentId<T>();
        flags[id / COMPONENT_FLAG_DATA_BITS] &= ~(static_cast<ComponentFlagDataType>(1) << id % COMPONENT_FLAG_DATA_BITS);
        if constexpr (sizeof...(Args) > 0)
            ClearFlags<Args...>();
    }

    template<typename T, typename...Args>
    void SetFlags () {
        const auto id = GetComponentId<T>();
        flags[id / COMPONENT_FLAG_DATA_BITS] |= static_cast<ComponentFlagDataType>(1) << id % COMPONENT_FLAG_DATA_BITS;
        if constexpr (sizeof...(Args) > 0)
            SetFlags<Args...>();
    }

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

} // namespace ecs

namespace std {
    template <> struct hash<::ecs::ComponentFlags> {
        size_t operator() (const ::ecs::ComponentFlags & flags) const {
            return flags.GetHash();
        }
    };
};