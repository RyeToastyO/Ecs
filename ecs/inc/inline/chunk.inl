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
    // Give a valid pointer if a tag component is requested, but don't
    // bother looking it up in the component arrays since we didn't allocate memory for it
    if (std::is_empty<T>())
        return GetComponentFlags().Has<T>() ? reinterpret_cast<T*>(m_componentMemory) : nullptr;

    auto iter = m_componentArrays.find(GetComponentId<T>());
    if (iter == m_componentArrays.end())
        return nullptr;
    return reinterpret_cast<T*>(iter->second);
}

template<typename T>
inline T* Chunk::Find (uint32_t index) {
    if (index >= m_count)
        return nullptr;

    T* arrayStart = Find<T>();
    return arrayStart ? arrayStart + index : nullptr;
}

inline Chunk::Chunk (const Composition& composition)
    : m_composition(composition)
{
    AllocateComponentArrays(kDefaultChunkSize);
}

inline Chunk::~Chunk () {
    Clear();
}

inline void Chunk::AllocateComponentArrays (uint32_t capacity) {
    Clear();

    // Allocate all memory for this chunk as one allocation
    const ComponentInfo& componentInfo = m_composition.GetComponentInfo();
    m_componentArrays.reserve(componentInfo.DataComponentCount);
    m_componentMemory = new byte_t[componentInfo.TotalSize * capacity];
    m_capacity = capacity;

    assert(m_componentMemory);

    auto arrayStart = m_componentMemory;
    // Assign each component array their location in that allocation
    for (const auto& compId : GetComponentFlags()) {
        const auto size = m_composition.GetComponentSize(compId);
        if (size == 0)
            continue;

        m_componentArrays.emplace(compId, arrayStart);
        arrayStart += size * capacity;
    }
}

inline void Chunk::Resize (uint32_t capacity) {
    byte_t* newMemory = new byte_t[m_composition.GetComponentInfo().TotalSize * capacity];
    m_capacity = capacity;

    auto newArrayStart = newMemory;
    for (auto& compArray : m_componentArrays) {
        auto oldArrayStart = compArray.second;
        auto size = m_composition.GetComponentSize(compArray.first);

        memcpy(newArrayStart, oldArrayStart, size * std::min(m_count, m_capacity));

        compArray.second = newArrayStart;
        newArrayStart += size * m_capacity;
    }

    delete[] m_componentMemory;
    m_componentMemory = newMemory;
}

inline uint32_t Chunk::AllocateEntity () {
    if (m_count == m_capacity)
        Resize(m_capacity * kGrowthFactor);
    return m_count++;
}

inline uint32_t Chunk::CloneEntity (uint32_t index) {
    assert(index < m_count);
    uint32_t newIndex = AllocateEntity();

    for (auto& compArray : m_componentArrays) {
        auto arrayStart = compArray.second;
        auto size = m_composition.GetComponentSize(compArray.first);

        memcpy(arrayStart + newIndex * size, arrayStart + index * size, size);
    }

    return newIndex;
}

inline void Chunk::Clear () {
    m_count = 0;
    m_capacity = 0;

    if (m_componentMemory) {
        delete[] m_componentMemory;
        m_componentMemory = nullptr;
    }
    m_componentArrays.clear();
}

inline uint32_t Chunk::GetCapacity () const {
    return m_capacity;
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
    // Make space in the chunk we are moving to
    auto newIndex = to.AllocateEntity();

    // Copy all our data over
    for (auto& compIter : m_componentArrays) {
        auto newCompArray = to.m_componentArrays.find(compIter.first);
        if (newCompArray == to.m_componentArrays.end())
            continue;
        auto newData = newCompArray->second;
        auto size = m_composition.GetComponentSize(compIter.first);
        memcpy(newData + newIndex * size, compIter.second + from * size, size);
    }

    // Remove from this chunk
    RemoveEntity(from);

    // Return the new chunk index
    return newIndex;
}

inline void Chunk::RemoveEntity (uint32_t index) {
    if (index >= m_count)
        return;

    // Copy whatever was in our last slot to the removed index
    CopyTo(--m_count, index);
}

inline void Chunk::CopyTo (uint32_t from, uint32_t to) {
    if (from == to)
        return;

    for (const auto& compIter : m_componentArrays) {
        auto arrayStart = compIter.second;
        auto size = m_composition.GetComponentSize(compIter.first);
        memcpy(arrayStart + (to * size), arrayStart + (from * size), size);
    }
}

} // namespace impl
} // namespace ecs
