// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#include <cassert>

namespace ecs {
namespace impl {

template<typename T>
inline IComponentCollection* AllocComponentCollection () {
    return new TComponentCollection<T>();
}

inline const ComponentFlags& Composition::GetComponentFlags () const {
    return m_flags;
}

inline const ComponentCollectionFactory& Composition::GetComponentCollectionFactory () const {
    return m_componentCollectionFactory;
}

inline size_t Composition::GetHash () const {
    return m_flags.GetHash();
}

inline bool Composition::operator== (const Composition& rhs) const {
    return m_flags == rhs.m_flags;
}

inline void Composition::Clear () {
    m_flags.Clear();
    m_componentCollectionFactory.clear();
}

template<typename T, typename...Args>
inline void Composition::RemoveComponents () {
    if (m_flags.Has<T>()) {
        m_flags.ClearFlags<T>();

        if (!std::is_empty<T>())
            m_componentCollectionFactory.erase(GetComponentId<T>());
    }
    RemoveComponents<Args...>();
    m_flags.ClearFlags<T, Args...>();
}

template<typename T, typename...Args>
inline void Composition::SetComponents (T component, Args...args) {
    SetComponentsInternal(component, args...);
}

inline void Composition::SetComponentsInternal () {};

template<typename T, typename...Args>
inline void Composition::SetComponentsInternal (T component, Args...args) {
    ECS_REF(component);
    if (!m_flags.Has<T>()) {
        m_flags.SetFlags<T>();

        if (!std::is_empty<T>())
            m_componentCollectionFactory.emplace(GetComponentId<T>(), &AllocComponentCollection<T>);
    }
    SetComponentsInternal(args...);
}

} // namespace impl
} // namespace ecs
