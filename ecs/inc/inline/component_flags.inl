/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "../helpers/hash.h"

namespace ecs {
namespace impl {

// ComponentFlagIterator
inline ComponentFlagIterator::ComponentFlagIterator (const ComponentFlags & flags)
    : m_flags(flags)
    , m_current(0) {
    if (!m_flags.Has(m_current))
        ++(*this);
}

inline ComponentFlagIterator::ComponentFlagIterator (const ComponentFlags & flags, ComponentId id)
    : m_flags(flags)
    , m_current(id) {
}

inline bool ComponentFlagIterator::operator!= (const ComponentFlagIterator & rhs) const {
    return m_current != rhs.m_current;
}
inline ComponentFlagIterator & ComponentFlagIterator::operator++ () {
    while (++m_current < ECS_MAX_COMPONENTS && !m_flags.Has(m_current));
    return *this;
}
inline const ComponentId & ComponentFlagIterator::operator* () const {
    return m_current;
}

inline ComponentFlagIterator ComponentFlagIterator::begin () const { return *this; }
inline ComponentFlagIterator ComponentFlagIterator::end () const { return ComponentFlagIterator(m_flags, ECS_MAX_COMPONENTS); }


// ComponentFlags
inline ComponentFlags::ComponentFlags () {
    Clear();
}

inline void ComponentFlags::Clear () {
    memset(flags, 0, COMPONENT_FLAG_DATA_COUNT * sizeof(ComponentFlagDataType));
}

inline void ComponentFlags::ClearFlag (ComponentId id) {
    flags[id / COMPONENT_FLAG_DATA_BITS] &= ~(static_cast<ComponentFlagDataType>(1) << id % COMPONENT_FLAG_DATA_BITS);
}

template<typename T, typename...Args>
inline void ComponentFlags::ClearFlags () {
    ClearFlag(GetComponentId<T>());
    ClearFlags<Args...>();
}

inline void ComponentFlags::ClearFlags (const ComponentFlags & rhs) {
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i)
        flags[i] &= ~rhs.flags[i];
}

inline void ComponentFlags::SetFlag (ComponentId id) {
    flags[id / COMPONENT_FLAG_DATA_BITS] |= static_cast<ComponentFlagDataType>(1) << id % COMPONENT_FLAG_DATA_BITS;
}

template<typename T, typename...Args>
inline void ComponentFlags::SetFlags () {
    SetFlag(GetComponentId<T>());
    SetFlags<Args...>();
}

inline void ComponentFlags::SetFlags (const ComponentFlags & rhs) {
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i)
        flags[i] |= rhs.flags[i];
}

inline ComponentInfo ComponentFlags::GetComponentInfo () const {
    ComponentInfo ret;
    auto iter = GetIterator();
    for (const auto & compId : iter) {
        const auto size = GetComponentSize(compId);
        ret.ComponentCount++;
        ret.TotalSize += size;
        ret.DataComponentCount += size > 0 ? 1 : 0;
    }
    return ret;
}

inline size_t ComponentFlags::GetHash () const {
    size_t hash = 0;
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i)
        HashCombine(hash, flags[i]);
    return hash;
}

inline ComponentFlagIterator ComponentFlags::GetIterator () const {
    return ComponentFlagIterator(*this);
}

inline bool ComponentFlags::HasAll (const ComponentFlags & rhs) const {
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i) {
        if ((flags[i] & rhs.flags[i]) != rhs.flags[i])
            return false;
    }
    return true;
}

inline bool ComponentFlags::HasAny (const ComponentFlags & rhs) const {
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i) {
        if (flags[i] & rhs.flags[i])
            return true;
    }
    return false;
}

inline bool ComponentFlags::HasNone (const ComponentFlags & rhs) const {
    return !HasAny(rhs);
}

inline bool ComponentFlags::Has (ComponentId id) const {
    return !!(flags[id / COMPONENT_FLAG_DATA_BITS] & static_cast<ComponentFlagDataType>(1) << id % COMPONENT_FLAG_DATA_BITS);
}

inline bool ComponentFlags::operator== (const ComponentFlags & rhs) const {
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i) {
        if (flags[i] != rhs.flags[i])
            return false;
    }
    return true;
}

} // namespace impl
} // namespace ecs
