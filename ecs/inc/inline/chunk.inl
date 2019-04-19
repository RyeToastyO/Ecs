#include <algorithm>
#include <cassert>

namespace ecs {

template<typename T>
T * Chunk::Find () {
    // Give a valid pointer if a tag component is requested, but don't
    // bother looking it up in the component arrays since we didn't allocate memory for it
    if (std::is_empty<T>())
        return m_composition.Has<T>() ? reinterpret_cast<T*>(m_componentMemory) : nullptr;

    auto iter = m_componentArrays.find(GetComponentId<T>());
    if (iter == m_componentArrays.end())
        return nullptr;
    return reinterpret_cast<T*>(iter->second);
}

template<typename T>
T * Chunk::Find (uint32_t index) {
    if (index >= m_count)
        return nullptr;

    T * arrayStart = Find<T>();
    return arrayStart ? arrayStart + index : nullptr;
}

Chunk::Chunk (const ComponentFlags & composition)
    : m_composition(composition)
    , m_componentInfo(composition.GetComponentInfo())
{
    AllocateComponentArrays(kDefaultChunkSize);
}

Chunk::~Chunk () {
    Clear();
}

void Chunk::AllocateComponentArrays (uint32_t capacity) {
    Clear();

    // Allocate all memory for this chunk as one allocation
    m_componentArrays.reserve(m_componentInfo.DataComponentCount);
    m_componentMemory = static_cast<byte_t*>(malloc(m_componentInfo.TotalSize * capacity));
    m_capacity = capacity;

    assert(m_componentMemory);

    auto arrayStart = m_componentMemory;
    // Assign each component array their location in that allocation
    auto iter = m_composition.GetIterator();
    for (const auto & compId : iter) {
        const auto size = GetComponentSize(compId);
        if (size == 0)
            continue;

        m_componentArrays.emplace(compId, arrayStart);
        arrayStart += size * capacity;
    }
}

void Chunk::Resize (uint32_t capacity) {
    byte_t * newMemory = static_cast<byte_t*>(malloc(m_componentInfo.TotalSize * capacity));
    m_capacity = capacity;

    auto newArrayStart = newMemory;
    for (auto & compArray : m_componentArrays) {
        auto oldArrayStart = compArray.second;
        auto size = GetComponentSize(compArray.first);

        memcpy(newArrayStart, oldArrayStart, size * std::min(m_count, m_capacity));

        compArray.second = newArrayStart;
        newArrayStart += size * m_capacity;
    }

    free(m_componentMemory);
    m_componentMemory = newMemory;
}

uint32_t Chunk::AllocateEntity () {
    if (m_count == m_capacity)
        Resize(m_capacity * kGrowthFactor);
    return m_count++;
}

void Chunk::Clear () {
    m_count = 0;
    m_capacity = 0;

    if (m_componentMemory) {
        free(m_componentMemory);
        m_componentMemory = nullptr;
    }
    m_componentArrays.clear();
}

uint32_t Chunk::GetCapacity () const {
    return m_capacity;
}

const ComponentFlags & Chunk::GetComposition () const {
    return m_composition;
}

uint32_t Chunk::GetCount () const {
    return m_count;
}

uint32_t Chunk::MoveTo (uint32_t from, Chunk & to) {
    // Make space in the chunk we are moving to
    auto newIndex = to.AllocateEntity();

    // Copy all our data over
    for (auto & compIter : m_componentArrays) {
        auto newCompArray = to.m_componentArrays.find(compIter.first);
        if (newCompArray == to.m_componentArrays.end())
            continue;
        auto newData = newCompArray->second;
        auto size = GetComponentSize(compIter.first);
        memcpy(newData + newIndex * size, compIter.second + from * size, size);
    }

    // Remove from this chunk
    RemoveEntity(from);

    // Return the new chunk index
    return newIndex;
}

void Chunk::RemoveEntity (uint32_t index) {
    if (index >= m_count)
        return;

    // Copy whatever was in our last slot to the removed index
    CopyTo(--m_count, index);
}

void Chunk::CopyTo (uint32_t from, uint32_t to) {
    if (from == to)
        return;

    for (const auto & compIter : m_componentArrays) {
        auto arrayStart = compIter.second;
        auto size = GetComponentSize(compIter.first);
        memcpy(arrayStart + (to * size), arrayStart + (from * size), size);
    }
}

}