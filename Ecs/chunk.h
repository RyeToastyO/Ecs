#pragma once

#include "composition.h"

namespace ecs {

struct ComponentArray {
    byte_t * m_data;
    size_t m_componentSize;

    template<typename T>
    T * As () { return reinterpret_cast<T*>(m_data); }
};

struct Chunk {
    Chunk (const ComponentFlags & composition);
    ~Chunk ();

    static const uint32_t kDefaultChunkSize = 1;
    static const uint32_t kGrowthFactor = 4;

    uint32_t GetCount () const;
    uint32_t GetCapacity () const;
    const ComponentFlags & GetComposition () const;

    template<typename T>
    T * Find ();
    template<typename T>
    T * Find (uint32_t index);

    uint32_t AllocateEntity ();
    uint32_t MoveTo (uint32_t from, Chunk & to);
    void RemoveEntity (uint32_t index);

private: // Data
    std::unordered_map<ComponentId, ComponentArray> m_componentArrays;
    void * m_componentMemory = nullptr;

    uint32_t m_count = 0;
    uint32_t m_capacity = 0;
    ComponentFlags m_composition;
    ComponentInfo m_componentInfo;

private: // Helpers
    void AllocateComponentArrays (uint32_t capacity);
    void Clear ();
    void CopyTo (uint32_t from, uint32_t to);
    void Resize (uint32_t capacity);
};

}

#include "chunk.inl"
