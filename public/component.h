#pragma once

#include <cstdint>
#include <type_traits>

namespace ecs {

typedef uint32_t ComponentId;

ComponentId AllocateComponentId ();
size_t GetComponentSize (ComponentId id);
size_t RegisterComponentSize (ComponentId id, size_t size);

#define DEFINE_ECS_COMPONENT(component)                                                         \
::ecs::ComponentId s_##component##Id = ::ecs::AllocateComponentId();                            \
::ecs::ComponentId component::GetId () { return s_##component##Id; }                            \
size_t component::SizeOf = ::ecs::RegisterComponentSize(component::GetId(), ::std::is_empty<component>() ? 0 : sizeof(component));

#define ECS_COMPONENT()                 \
public:                                 \
    static ::ecs::ComponentId GetId (); \
    static size_t SizeOf;

#define ECS_TAG_COMPONENT(component) struct component { ECS_COMPONENT() }

} // namespace ecs
