/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "../ecs/ecs.h"

// Test components
namespace test {

struct TagA {};
struct TagB {};
struct TagC {};

struct EntityReference { ecs::Entity Value; };

struct DoubleA { double Value = 0.0; };
struct DoubleB { double Value = 0.0; };
struct DoubleC { double Value = 0.0; };

struct FloatA { float Value = 0.0f; };
struct FloatB { float Value = 0.0f; };
struct FloatC { float Value = 0.0f; };

struct IntA { int32_t Value = 0; };
struct IntB { int32_t Value = 0; };
struct IntC { int32_t Value = 0; };

struct UintA { uint32_t Value = 0; };
struct UintB { uint32_t Value = 0; };
struct UintC { uint32_t Value = 0; };

struct SingletonDouble : ecs::ISingletonComponent { double Value = 0.0; };
struct SingletonFloat : ecs::ISingletonComponent { float Value = 0.0f; };
struct SingletonInt : ecs::ISingletonComponent { int32_t Value = 0; };
struct SingletonUint : ecs::ISingletonComponent { uint32_t Value = 0; };

struct SharedA : ecs::ISharedComponent {
    SharedA (uint32_t value) : Value(value) {}
    uint32_t Value;
};
struct SharedB : ecs::ISharedComponent {
    SharedB (uint32_t value) : Value(value) {}
    uint32_t Value;
};

extern std::shared_ptr<SharedA> SharedA1;
extern std::shared_ptr<SharedA> SharedA2;
extern std::shared_ptr<SharedB> SharedB1;
extern std::shared_ptr<SharedB> SharedB2;

} // namespace test
