/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "composition.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ecs {

typedef uint8_t byte_t;

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
    std::unordered_map<ComponentId, byte_t*> m_componentArrays;
    byte_t * m_componentMemory = nullptr;

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

#include "inline/chunk.inl"
