// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include "composition.h"

#include <cstdint>
#include <unordered_map>

namespace ecs {
namespace impl {

typedef uint8_t byte_t;

struct Chunk {
    Chunk (const Composition& composition);
    ~Chunk ();

    static const uint32_t kDefaultChunkSize = 1;
    static const uint32_t kGrowthFactor = 4;

    uint32_t GetCount () const;
    uint32_t GetCapacity () const;
    const Composition& GetComposition () const;
    const ComponentFlags& GetComponentFlags () const;

    template<typename T>
    T* Find ();
    template<typename T>
    T* Find (uint32_t index);

    uint32_t AllocateEntity ();
    uint32_t CloneEntity (uint32_t index);
    uint32_t MoveTo (uint32_t from, Chunk& to);
    void RemoveEntity (uint32_t index);

private: // Data
    std::unordered_map<ComponentId, byte_t*> m_componentArrays;
    byte_t* m_componentMemory = nullptr;

    uint32_t m_count = 0;
    uint32_t m_capacity = 0;
    Composition m_composition;

private: // Helpers
    void AllocateComponentArrays (uint32_t capacity);
    void Clear ();
    void CopyTo (uint32_t from, uint32_t to);
    void Resize (uint32_t capacity);
};

} // namespace impl
} // namespace ecs
