// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

namespace ecs {
namespace impl {

inline const ComponentFlags& Composition::GetComponentFlags () const {
    return m_flags;
}

inline size_t Composition::GetHash () const {
    return m_flags.GetHash();
}

inline bool Composition::operator== (const Composition& rhs) const {
    return m_flags == rhs.m_flags;
}

template<typename T, typename...Args>
inline void Composition::RemoveComponents () {
    m_flags.ClearFlags<T, Args...>();
}

template<typename T, typename...Args>
inline void Composition::SetComponents (T component, Args...args) {
    SetComponentsInternal(component, args...);
}

inline void Composition::SetComponentsInternal () {};

template<typename T, typename...Args>
inline void Composition::SetComponentsInternal (std::shared_ptr<T> component, Args...args) {
    m_flags.SetFlags<T>();
    SetComponentsInternal(args...);
}

template<typename T, typename...Args>
inline void Composition::SetComponentsInternal (T component, Args...args) {
    ECS_REF(component);
    m_flags.SetFlags<T>();
    SetComponentsInternal(args...);
}

} // namespace impl
} // namespace ecs
