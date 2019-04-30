/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "../ecs/ecs.h"

namespace test {

const auto MULTI_THREAD_ENTITY_COUNT = 25000;

enum EThreadingType : uint8_t {
    Single,
    ManualMulti,
    UpdateGroupMulti
};

void InitMultiThreadingTest (ecs::Manager * mgr);
void ExecuteMultiThreadingTest (ecs::Manager * mgr, EThreadingType threading);
void TestManualMultiThreading ();
void TestMultiThreading ();
void TestMultipleManagers ();

}
