#include "pch.h"

#include "composition.h"

namespace ecs {

// ComponentFlagIterator
ComponentFlagIterator::ComponentFlagIterator (const ComponentFlags & flags)
    : m_flags(flags)
    , m_current(0)
{
    if (!m_flags.Has(m_current))
        ++(*this);
}

ComponentFlagIterator::ComponentFlagIterator (const ComponentFlags & flags, ComponentId id)
    : m_flags(flags)
    , m_current(id)
{
}

bool ComponentFlagIterator::operator!= (const ComponentFlagIterator & rhs) const {
    return m_current != rhs.m_current;
}
ComponentFlagIterator & ComponentFlagIterator::operator++ () {
    while (++m_current < MAX_ECS_COMPONENTS && !m_flags.Has(m_current));
    return *this;
}
const ComponentId & ComponentFlagIterator::operator* () const {
    return m_current;
}

ComponentFlagIterator ComponentFlagIterator::begin () const { return *this; }
ComponentFlagIterator ComponentFlagIterator::end () const { return ComponentFlagIterator(m_flags, MAX_ECS_COMPONENTS); }


// ComponentFlags
ComponentFlags::ComponentFlags () {
    Clear();
}

void ComponentFlags::Clear () {
    memset(flags, 0, COMPONENT_FLAG_DATA_COUNT * sizeof(ComponentFlagDataType));
}

ComponentInfo ComponentFlags::GetComponentInfo () const {
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

size_t ComponentFlags::GetHash () const {
    size_t hash = 0;
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i)
        hash_combine(hash, flags[i]);
    return hash;
}

ComponentFlagIterator ComponentFlags::GetIterator () const {
    return ComponentFlagIterator(*this);
}

bool ComponentFlags::HasAll (const ComponentFlags & rhs) const {
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i) {
        if ((flags[i] & rhs.flags[i]) != rhs.flags[i])
            return false;
    }
    return true;
}

bool ComponentFlags::HasAny (const ComponentFlags & rhs) const {
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i) {
        if (flags[i] & rhs.flags[i])
            return true;
    }
    return false;
}

bool ComponentFlags::HasNone (const ComponentFlags & rhs) const {
    return !HasAny(rhs);
}

bool ComponentFlags::Has (ComponentId id) const {
    return !!(flags[id / COMPONENT_FLAG_DATA_BITS] & static_cast<ComponentFlagDataType>(1) << id % COMPONENT_FLAG_DATA_BITS);
}

bool ComponentFlags::operator== (const ComponentFlags & rhs) const {
    for (auto i = 0; i < COMPONENT_FLAG_DATA_COUNT; ++i) {
        if (flags[i] != rhs.flags[i])
            return false;
    }
    return true;
}

} // namespace ecs
