/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

namespace ecs {
namespace impl {

inline const ComponentFlags & Composition::GetComponentFlags () const {
    return m_flags;
}

inline const std::map<ComponentId, ISharedComponentPtr> & Composition::GetSharedComponents () const {
    return m_shared;
}

inline size_t Composition::GetHash () const {
    // Start with the hash of our component flags
    size_t hash = m_flags.GetHash();

    // Apply our ordered shared component values
    auto iter = m_shared.begin();
    while (iter != m_shared.end()) {
        HashCombine(hash, iter->second);
        ++iter;
    }

    return hash;
}

inline bool Composition::operator== (const Composition & rhs) const {
    if (m_shared.size() != rhs.m_shared.size())
        return false;
    if (!(m_flags == rhs.m_flags))
        return false;

    auto iterLhs = m_shared.begin();
    auto iterRhs = rhs.m_shared.begin();
    while (iterLhs != m_shared.end()) {
        if (!(iterLhs->second == iterRhs->second))
            return false;
        ++iterLhs;
        ++iterRhs;
    }

    return true;
}

template<typename T, typename...Args>
inline void Composition::RemoveComponents () {
    m_flags.ClearFlags<T, Args...>();
    RemoveSharedComponentsInternal<T, Args...>();
}

template<typename T, typename...Args>
inline void Composition::RemoveSharedComponentsInternal () {
    if (std::is_base_of<ISharedComponent, T>::value)
        m_shared.erase(GetComponentId<T>());
    RemoveSharedComponentsInternal<Args...>();
}

template<typename T, typename...Args>
inline void Composition::SetComponents (T component, Args...args) {
    SetComponentsInternal(component, args...);
}

inline void Composition::SetComponentsInternal () {};

template<typename T, typename...Args>
inline void Composition::SetComponentsInternal (std::shared_ptr<T> component, Args...args) {
    m_flags.SetFlags<T>();
    m_shared.erase(GetComponentId<T>());
    m_shared.emplace(GetComponentId<T>(), component);
    SetComponentsInternal(args...);
}

template<typename T, typename...Args>
inline void Composition::SetComponentsInternal (T component, Args...args) {
    static_assert(!std::is_base_of<ISharedComponent, T>::value, "Don't directly add shared components, must be a std::shared_ptr<ComponentType>");

    ECS_REF(component);
    m_flags.SetFlags<T>();
    SetComponentsInternal(args...);
}

} // namespace impl
} // namespace ecs
