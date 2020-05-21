/*
 * Copyright (c) 2020 Riley Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "test_multi_threading.h"
#include "test_components.h"
#include "tests.h"

#include "../ecs/ecs.h"

namespace test {

void SpeedTestCalculation (float dt, float & a, const float & b, const float & c) {
    a = std::min(a + c * dt, b);
}

struct SpeedTestJob : public ecs::Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_READ(test::FloatC, C);
    ECS_EXCLUDE(test::TagA);

    ECS_READ_SINGLETON(test::DeltaTime, Dt);

    void ForEach () override {
        SpeedTestCalculation(Dt->Value, A->Value, B->Value, C->Value);
    }
};

void TestJobSpeed () {
    ecs::Manager mgr;

    // Run once with no entities to initialize the job outside our profiling
    mgr.RunJob<SpeedTestJob>();

    uint32_t entityCount = 100000;
    uint32_t loopCount = 1 * 60 * 60;

    for (uint32_t i = 0; i < entityCount; ++i)
        mgr.CreateEntityImmediate(test::FloatA{ 0 }, test::FloatB{ 1000 }, test::FloatC{ 1.0f });

    constexpr float timestep = 1 / 60.0f;
    mgr.GetSingletonComponent<test::DeltaTime>()->Value = timestep;

    auto start = std::chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < loopCount; ++i)
        mgr.RunJob<SpeedTestJob>();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedJob = end - start;

    uint64_t benchmarkLoops = entityCount * (uint64_t)loopCount;
    float a = 0;
    float b = 1000;
    float c = 1;
    start = std::chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < benchmarkLoops; ++i)
        SpeedTestCalculation(timestep, a, b, c);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedBenchmark = end - start;

    double maxRatio = 2.0;
    EXPECT_FALSE(elapsedJob.count() > elapsedBenchmark.count() * maxRatio);
    if (elapsedJob.count() > elapsedBenchmark.count() * maxRatio)
        std::cout << "  " << elapsedJob.count() * 1000 << "ms vs " << elapsedBenchmark.count() * 1000 << "ms (" << 100 * elapsedJob.count() / elapsedBenchmark.count() << "%)" << std::endl;
}

void TestMultiThreadingSpeed () {
    auto loopCount = 1 * 60 * 60;

    std::chrono::duration<double> elapsedManual;
    {
        ecs::Manager mgr;
        InitMultiThreadingTest(&mgr);

        auto start = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < loopCount; ++i)
            ExecuteMultiThreadingTest(&mgr, EThreadingType::ManualMulti);
        auto end = std::chrono::high_resolution_clock::now();
        elapsedManual = end - start;
    }

    std::chrono::duration<double> elapsedSingle;
    {
        ecs::Manager mgr;
        InitMultiThreadingTest(&mgr);

        auto start = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < loopCount; ++i)
            ExecuteMultiThreadingTest(&mgr, EThreadingType::Single);
        auto end = std::chrono::high_resolution_clock::now();
        elapsedSingle = end - start;
    }

    double maxRatio = 1.0;
    EXPECT_FALSE(elapsedManual.count() > elapsedSingle.count() * maxRatio);
    if (elapsedManual.count() > elapsedSingle.count() * maxRatio) {
        std::cout << "  " << elapsedManual.count() * 1000 << "ms vs " << elapsedSingle.count() * 1000 << "ms (" << 100 * elapsedManual.count() / elapsedSingle.count() << "%)" << std::endl;
    }
}

void TestSpeed () {
#ifndef _DEBUG
    TestJobSpeed();
    TestMultiThreadingSpeed();
#endif
}

}
