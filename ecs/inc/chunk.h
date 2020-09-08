// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include "composition.h"
#include "component_collection.h"

#include <cstdint>
#include <unordered_map>

namespace ecs {
namespace impl {

typedef uint8_t byte_t;

struct Chunk {
    Chunk (const Composition& composition);
    ~Chunk ();

    uint32_t GetCount () const;
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
    std::unordered_map<ComponentId, IComponentCollection*> m_componentArrays;

    uint32_t m_count = 0;
    Composition m_composition;
};

} // namespace impl
} // namespace ecs
