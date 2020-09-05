// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

namespace ecs {
namespace impl {

// ComponentFlags
inline ComponentFlags::ComponentFlags () {
    Clear();
}

inline void ComponentFlags::Clear () {
    m_flags.clear();
}

inline void ComponentFlags::ClearFlag (ComponentId id) {
    m_flags.erase(id);
}

template<typename T, typename...Args>
inline void ComponentFlags::ClearFlags () {
    ClearFlag(GetComponentId<T>());
    ClearFlags<Args...>();
}

inline void ComponentFlags::ClearFlags (const ComponentFlags& rhs) {
    for (const ComponentId& id : rhs.m_flags)
        ClearFlag(id);
}

inline void ComponentFlags::SetFlag (ComponentId id) {
    m_flags.emplace(id);
}

template<typename T, typename...Args>
inline void ComponentFlags::SetFlags () {
    SetFlag(GetComponentId<T>());
    SetFlags<Args...>();
}

inline void ComponentFlags::SetFlags (const ComponentFlags& rhs) {
    for (const ComponentId& id : rhs.m_flags)
        SetFlag(id);
}

inline ComponentFlagIterator ComponentFlags::begin () const {
    return m_flags.begin();
}

inline ComponentFlagIterator ComponentFlags::end () const {
    return m_flags.end();
}

inline size_t ComponentFlags::GetHash () const {
    // Since component ids are already hashes themselves, and we don't want
    // order to matter, we can just add all of the flags together to combine
    ComponentId hash = 0;
    for (const ComponentId& id : m_flags)
        hash += id;
    return (size_t)hash;
}

inline bool ComponentFlags::HasAll (const ComponentFlags& rhs) const {
    for (const ComponentId& id : rhs.m_flags) {
        if (!Has(id))
            return false;
    }
    return true;
}

inline bool ComponentFlags::HasAny (const ComponentFlags& rhs) const {
    for (const ComponentId& id : rhs.m_flags) {
        if (Has(id))
            return true;
    }
    return false;
}

inline bool ComponentFlags::HasNone (const ComponentFlags& rhs) const {
    return !HasAny(rhs);
}

inline bool ComponentFlags::Has (ComponentId id) const {
    return m_flags.find(id) != m_flags.end();
}

inline bool ComponentFlags::operator== (const ComponentFlags& rhs) const {
    return m_flags == rhs.m_flags;
}

} // namespace impl
} // namespace ecs
