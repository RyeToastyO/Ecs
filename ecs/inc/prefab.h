// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include "entity.h"

namespace ecs {

namespace impl {
struct CommandQueue;
} // namespace impl

// - Prefabs hold a composition and default values for those components
// - Call Manager->CreatePrefab(Components...) to make one
// - Call Manager->SpawnPrefab(Prefab) to create an entity from a prefab
struct Prefab {
    Prefab () {}
private:
    Prefab (Entity entity) : m_entity(entity) {}
    Entity m_entity;

    friend class Manager;
    friend struct impl::CommandQueue;
};

namespace impl {

struct PrefabComponent {};

} // namespace impl
} // namespace ecs
