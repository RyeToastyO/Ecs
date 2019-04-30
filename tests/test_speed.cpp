/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "test_multi_threading.h"
#include "test_components.h"
#include "tests.h"

#include "../ecs/ecs.h"

namespace test {

void SpeedTestCalculation (ecs::Timestep dt, float & a, const float & b, const float & c) {
    a = std::min(a + c * dt, b);
}

struct SpeedTestJob : public ecs::Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_READ(test::FloatC, C);
    ECS_EXCLUDE(test::TagA);

    void ForEach (ecs::Timestep dt) override {
        SpeedTestCalculation(dt, A->Value, B->Value, C->Value);
    }
};

void TestJobSpeed () {
    ecs::Manager mgr;

    // Run once with no entities to initialize the job outside our profiling
    mgr.RunJob<SpeedTestJob>(0.0f);

    uint32_t entityCount = 100000;
    uint32_t loopCount = 1 * 60 * 60;

    for (uint32_t i = 0; i < entityCount; ++i)
        mgr.CreateEntityImmediate(test::FloatA{ 0 }, test::FloatB{ 1000 }, test::FloatC{ 1.0f });

    float timestep = 1 / 60.0f;
    auto start = std::chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < loopCount; ++i)
        mgr.RunJob<SpeedTestJob>(timestep);
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

    std::chrono::duration<double> elapsedMulti;
    {
        ecs::Manager mgr;
        InitMultiThreadingTest(&mgr);

        auto start = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < loopCount; ++i)
            ExecuteMultiThreadingTest(&mgr, EThreadingType::UpdateGroupMulti);
        auto end = std::chrono::high_resolution_clock::now();
        elapsedMulti = end - start;
    }

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
    EXPECT_FALSE(elapsedMulti.count() > elapsedSingle.count() * maxRatio);
    if (elapsedMulti.count() > elapsedSingle.count() * maxRatio) {
        std::cout << "  " << elapsedMulti.count() * 1000 << "ms vs " << elapsedSingle.count() * 1000 << "ms (" << 100 * elapsedMulti.count() / elapsedSingle.count() << "%)" << std::endl;
        std::cout << "  (Manual Multithreading: " << elapsedManual.count() * 1000 << "ms)" << std::endl;
    }
}

void TestSpeed () {
#ifndef _DEBUG
    TestJobSpeed();
    TestMultiThreadingSpeed();
#endif
}

}
