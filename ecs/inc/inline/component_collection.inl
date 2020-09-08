// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#include <cassert>

namespace ecs {
namespace impl {

template<typename T>
inline T* IComponentCollection::Get (uint32_t index) {
    assert(::ecs::impl::GetComponentId<T>() == GetComponentId());
    return static_cast<T*>(GetComponentAtIndex(index));
}

template<typename T>
uint32_t TComponentCollection<T>::Allocate () {
    m_components.push_back(T());
    return m_components.size() - 1;
}

template<typename T>
void TComponentCollection<T>::CopyTo (uint32_t from, uint32_t to) {
    m_components[to] = m_components[from];
}

template<typename T>
void TComponentCollection<T>::MoveTo (uint32_t fromIndex, IComponentCollection& to, uint32_t toIndex) {
    std::swap(m_components[fromIndex], *to.Get<T>(toIndex));
}

template<typename T>
void TComponentCollection<T>::Remove (uint32_t index) {
    std::swap(m_components[index], m_components[m_components.size() - 1]);
    m_components.pop_back();
}

template<typename T>
void TComponentCollection<T>::RemoveAll () {
    m_components.clear();
}

template<typename T>
void* TComponentCollection<T>::GetComponentAtIndex (uint32_t index) {
    return &m_components[index];
}

template<typename T>
ComponentId TComponentCollection<T>::GetComponentId () const {
    return ::ecs::impl::GetComponentId<T>();
}

} // namespace impl
} // namespace ecs
