/*
 * Copyright (c) 2020 Riley Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "tests.h"
#include "test_components.h"
#include "test_correctness.h"
#include "test_multi_threading.h"

#include <future>

#include "../ecs/ecs.h"

namespace test {

#define MULTI_THREAD_JOB(Type)                                                      \
struct MultiThreadJob##Type : public ::ecs::Job {                                   \
    ECS_READ(::test::Type##A, A);                                                   \
    ECS_READ(::test::Type##B, B);                                                   \
    ECS_WRITE(::test::Type##C, C);                                                  \
                                                                                    \
    void ForEach (::ecs::Timestep) override {                                       \
        C->Value = std::max(A->Value, B->Value);                                    \
    }                                                                               \
};

#define MULTI_THREAD_SINGLETON_JOB(Type)                                            \
struct MultiThreadSingletonJob##Type : public ::ecs::Job {                          \
    ECS_READ(::test::Type##C, C);                                                   \
    ECS_WRITE_SINGLETON(::test::Singleton##Type, Total);                            \
                                                                                    \
    void Run (::ecs::Timestep dt) override {                                        \
        Total->Value = 0;                                                           \
        ::ecs::Job::Run(dt);                                                        \
    }                                                                               \
                                                                                    \
    void ForEach (::ecs::Timestep) override {                                       \
        Total->Value += C->Value;                                                   \
    }                                                                               \
};

MULTI_THREAD_JOB(Double);
MULTI_THREAD_JOB(Float);
MULTI_THREAD_JOB(Int);
MULTI_THREAD_JOB(Uint);

MULTI_THREAD_SINGLETON_JOB(Double);
MULTI_THREAD_SINGLETON_JOB(Float);
MULTI_THREAD_SINGLETON_JOB(Int);
MULTI_THREAD_SINGLETON_JOB(Uint);

#define MULTI_THREAD_ENTITY_COUNT 25000

void InitMultiThreadingTest (ecs::Manager * mgr) {
    // Run these first to eliminate the first time init costs
    mgr->RunJob<MultiThreadJobDouble>(0.0f);
    mgr->RunJob<MultiThreadJobFloat>(0.0f);
    mgr->RunJob<MultiThreadJobInt>(0.0f);
    mgr->RunJob<MultiThreadJobUint>(0.0f);
    mgr->RunJob<MultiThreadSingletonJobDouble>(0.0f);
    mgr->RunJob<MultiThreadSingletonJobFloat>(0.0f);
    mgr->RunJob<MultiThreadSingletonJobInt>(0.0f);
    mgr->RunJob<MultiThreadSingletonJobUint>(0.0f);

    for (auto i = 0; i < MULTI_THREAD_ENTITY_COUNT; ++i) {
        mgr->CreateEntityImmediate(
            test::DoubleA{ 1.0 }, test::DoubleB{ 2.0 }, test::DoubleC{ 1.0 },
            test::FloatA{ 1.0f }, test::FloatB{ 2.0f }, test::FloatC{ 1.0f },
            test::IntA{ 1 }, test::IntB{ 2 }, test::IntC{ 1 },
            test::UintA{ 1 }, test::UintB{ 2 }, test::UintC{ 1 }
        );
    }
}

void ExecuteMultiThreadingTest (ecs::Manager * mgr, EThreadingType threading) {
    switch (threading) {
        case EThreadingType::Single: {
            mgr->RunJob<MultiThreadJobDouble>(0.0f);
            mgr->RunJob<MultiThreadSingletonJobDouble>(0.0);
            mgr->RunJob<MultiThreadJobFloat>(0.0f);
            mgr->RunJob<MultiThreadSingletonJobFloat>(0.0);
            mgr->RunJob<MultiThreadJobInt>(0.0f);
            mgr->RunJob<MultiThreadSingletonJobInt>(0.0);
            mgr->RunJob<MultiThreadJobUint>(0.0f);
            mgr->RunJob<MultiThreadSingletonJobUint>(0.0);
        } break;
        case EThreadingType::ManualMulti: {
            std::future<void> handle[4];
            handle[0] = std::async(std::launch::async, [mgr]() {
                mgr->RunJob<MultiThreadJobDouble>(0.0f);
                mgr->RunJob<MultiThreadSingletonJobDouble>(0.0);
            });
            handle[1] = std::async(std::launch::async, [mgr]() {
                mgr->RunJob<MultiThreadJobFloat>(0.0f);
                mgr->RunJob<MultiThreadSingletonJobFloat>(0.0);
            });
            handle[2] = std::async(std::launch::async, [mgr]() {
                mgr->RunJob<MultiThreadJobInt>(0.0f);
                mgr->RunJob<MultiThreadSingletonJobInt>(0.0);
            });
            handle[3] = std::async(std::launch::async, [mgr]() {
                mgr->RunJob<MultiThreadJobUint>(0.0f);
                mgr->RunJob<MultiThreadSingletonJobUint>(0.0);
            });
            for (auto i = 0; i < 4; ++i)
                handle[i].wait();
        } break;
    }
    EXPECT_TRUE(mgr->GetSingletonComponent<test::SingletonDouble>()->Value == 2.0 * MULTI_THREAD_ENTITY_COUNT);
    EXPECT_TRUE(mgr->GetSingletonComponent<test::SingletonFloat>()->Value == 2.0f * MULTI_THREAD_ENTITY_COUNT);
    EXPECT_TRUE(mgr->GetSingletonComponent<test::SingletonInt>()->Value == 2 * MULTI_THREAD_ENTITY_COUNT);
    EXPECT_TRUE(mgr->GetSingletonComponent<test::SingletonUint>()->Value == 2 * MULTI_THREAD_ENTITY_COUNT);
}

void TestManualMultiThreading () {
    ecs::Manager mgr;
    InitMultiThreadingTest(&mgr);
    ExecuteMultiThreadingTest(&mgr, EThreadingType::ManualMulti);
}

void TestMultipleManagers () {
    const auto threadCount = 4;
    std::future<void> threads[threadCount];

    for (auto i = 0; i < threadCount; ++i)
        threads[i] = std::async(std::launch::async, TestCorrectness);
    for (auto i = 0; i < threadCount; ++i)
        threads[i].wait();
}

}
