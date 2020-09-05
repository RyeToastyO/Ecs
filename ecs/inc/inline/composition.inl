// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#include <cassert>

namespace ecs {
namespace impl {

inline const ComponentFlags& Composition::GetComponentFlags () const {
    return m_flags;
}

inline const ComponentInfo& Composition::GetComponentInfo () const {
    return m_componentInfo;
}

inline size_t Composition::GetComponentSize (ComponentId id) const {
    auto iter = m_componentSizes.find(id);

    // This function is only valid for components contained in this composition
    assert(iter != m_componentSizes.end());

    return iter->second;
}

inline size_t Composition::GetHash () const {
    return m_flags.GetHash();
}

inline bool Composition::operator== (const Composition& rhs) const {
    return m_flags == rhs.m_flags;
}

inline void Composition::Clear () {
    m_flags.Clear();
    m_componentSizes.clear();
    m_componentInfo = ComponentInfo();
}

template<typename T, typename...Args>
inline void Composition::RemoveComponents () {
    if (m_flags.Has<T>()) {
        m_flags.ClearFlags<T>();

        size_t size = ::ecs::impl::GetComponentSize<T>();
        m_componentInfo.ComponentCount--;
        m_componentInfo.DataComponentCount -= size > 0 ? 1 : 0;
        m_componentInfo.TotalSize -= size;

        m_componentSizes.erase(GetComponentId<T>());
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

        size_t size = ::ecs::impl::GetComponentSize<T>();
        m_componentInfo.ComponentCount++;
        m_componentInfo.DataComponentCount += size > 0 ? 1 : 0;
        m_componentInfo.TotalSize += size;

        m_componentSizes.emplace(GetComponentId<T>(), size);
    }
    SetComponentsInternal(args...);
}

} // namespace impl
} // namespace ecs
