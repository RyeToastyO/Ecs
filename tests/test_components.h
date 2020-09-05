// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include "../ecs/ecs.h"

// Test components
namespace test {

struct TagA { ECS_COMPONENT(TagA) };
struct TagB { ECS_COMPONENT(TagB) };
struct TagC { ECS_COMPONENT(TagC) };

struct EntityReference { ECS_COMPONENT(EntityReference) ecs::Entity Value; };

struct DoubleA { ECS_COMPONENT(DoubleA) double Value = 0.0; };
struct DoubleB { ECS_COMPONENT(DoubleB) double Value = 0.0; };
struct DoubleC { ECS_COMPONENT(DoubleC) double Value = 0.0; };

struct FloatA { ECS_COMPONENT(FloatA) float Value = 0.0f; };
struct FloatB { ECS_COMPONENT(FloatB) float Value = 0.0f; };
struct FloatC { ECS_COMPONENT(FloatC) float Value = 0.0f; };

struct IntA { ECS_COMPONENT(IntA) int32_t Value = 0; };
struct IntB { ECS_COMPONENT(IntB) int32_t Value = 0; };
struct IntC { ECS_COMPONENT(IntC) int32_t Value = 0; };

struct UintA { ECS_COMPONENT(UintA) uint32_t Value = 0; };
struct UintB { ECS_COMPONENT(UintB) uint32_t Value = 0; };
struct UintC { ECS_COMPONENT(UintC) uint32_t Value = 0; };

struct SingletonDouble : ecs::ISingletonComponent { ECS_COMPONENT(SingletonDouble) double Value = 0.0; };
struct SingletonFloat : ecs::ISingletonComponent { ECS_COMPONENT(SingletonFloat) float Value = 0.0f; };
struct SingletonInt : ecs::ISingletonComponent { ECS_COMPONENT(SingletonInt) int32_t Value = 0; };
struct SingletonUint : ecs::ISingletonComponent { ECS_COMPONENT(SingletonUint) uint32_t Value = 0; };

struct DeltaTime : ecs::ISingletonComponent {
    ECS_COMPONENT(DeltaTime)
    float Value = 0.0f;
};

} // namespace test
