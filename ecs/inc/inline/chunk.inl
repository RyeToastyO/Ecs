// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#include <algorithm>
#include <cassert>

namespace ecs {
namespace impl {

template<typename T>
inline T* Chunk::Find () {
    return Find<T>(0);
}

template<typename T>
inline T* Chunk::Find (uint32_t index) {
    static_assert(!std::is_empty<T>(), "Tag components don't actually exist, cannot return pointer to one");

    if (index >= m_count)
        return nullptr;

    auto iter = m_componentArrays.find(GetComponentId<T>());
    if (iter == m_componentArrays.end())
        return nullptr;
    return iter->second->Get<T>(index);
}

inline Chunk::Chunk (const Composition& composition)
    : m_composition(composition)
{
    for (const auto& factoryIter : m_composition.GetComponentCollectionFactory())
        m_componentArrays.emplace(factoryIter.first, factoryIter.second());
}

inline Chunk::~Chunk () {
    m_count = 0;

    for (auto& compArray : m_componentArrays)
        delete compArray.second;
    m_componentArrays.clear();
}

inline uint32_t Chunk::AllocateEntity () {
    for (auto& compArray : m_componentArrays)
        compArray.second->Allocate();
    return m_count++;
}

inline uint32_t Chunk::CloneEntity (uint32_t index) {
    assert(index < m_count);
    uint32_t newIndex = AllocateEntity();

    for (auto& compIter : m_componentArrays) {
        IComponentCollection* compCollection = compIter.second;
        compCollection->CopyTo(index, newIndex);
    }

    return newIndex;
}

inline const Composition& Chunk::GetComposition () const {
    return m_composition;
}

inline const ComponentFlags& Chunk::GetComponentFlags () const {
    return m_composition.GetComponentFlags();
}

inline uint32_t Chunk::GetCount () const {
    return m_count;
}

inline uint32_t Chunk::MoveTo (uint32_t from, Chunk& to) {
    assert(from < m_count);

    // Make space in the chunk we are moving to
    auto newIndex = to.AllocateEntity();

    // Copy all our data over
    for (auto& compIter : m_componentArrays) {
        auto newCompArray = to.m_componentArrays.find(compIter.first);
        if (newCompArray == to.m_componentArrays.end())
            continue;
        IComponentCollection* fromCollection = compIter.second;
        IComponentCollection* toCollection = newCompArray->second;
        fromCollection->MoveTo(from, *toCollection, newIndex);
    }

    // Remove from this chunk
    RemoveEntity(from);

    // Return the new chunk index
    return newIndex;
}

inline void Chunk::RemoveEntity (uint32_t index) {
    if (index >= m_count)
        return;

    m_count--;
    for (auto& compIter : m_componentArrays) {
        IComponentCollection* compCollection = compIter.second;
        compCollection->Remove(index);
    }
}

} // namespace impl
} // namespace ecs
