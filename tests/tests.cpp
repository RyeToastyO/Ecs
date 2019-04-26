#include "../ecs/ecs.h"

#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>

using namespace ecs;

// Static data
int s_errorCount = 0;
std::mutex s_errorLock;

// Test components
namespace test {

struct TagA {};
struct TagB {};
struct TagC {};

struct EntityReference { Entity Value; };

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

struct SingletonDouble : ISingletonComponent { double Value = 0.0; };
struct SingletonFloat : ISingletonComponent { float Value = 0.0f; };
struct SingletonInt : ISingletonComponent { int32_t Value = 0; };
struct SingletonUint : ISingletonComponent { uint32_t Value = 0; };

}

// Helpers
#define EXPECT_TRUE(condition)                                                                  \
    if (!(condition)) {                                                                         \
        s_errorLock.lock();                                                                     \
        ++s_errorCount;                                                                         \
        std::cout << __FUNCTION__ << "(line " << __LINE__ << "): " << #condition << std::endl;  \
        s_errorLock.unlock();                                                                   \
    }
#define EXPECT_FALSE(condition) EXPECT_TRUE(!(condition));

// Tests
void TestAssumptions () {
    static_assert(sizeof(Entity) == (sizeof(uint32_t) + sizeof(uint32_t)), "Unexpected Entity size");
    static_assert(std::is_empty<test::TagA>(), "Unexpected tag size");
    static_assert(sizeof(test::FloatA) == sizeof(float), "Unexpected struct size");
}

void TestEntityComparison () {
    EXPECT_FALSE((Entity{ 1, 1 }) == (Entity{ 1, 2 }));
    EXPECT_FALSE((Entity{ 1, 1 }) == (Entity{ 2, 1 }));
    EXPECT_TRUE((Entity{ 1, 1 }) == (Entity{ 1, 1 }));
}

void TestEntityCreationDestruction () {
    Manager mgr;

    // Create and destroy a single entity
    {
        Entity entity = mgr.CreateEntityImmediate();

        EXPECT_TRUE(mgr.Exists(entity));

        mgr.DestroyImmediate(entity);

        EXPECT_FALSE(mgr.Exists(entity));
    }

    // Create several entities and destroy them in an odd order
    {
        Entity first = mgr.CreateEntityImmediate();
        Entity second = mgr.CreateEntityImmediate();
        Entity third = mgr.CreateEntityImmediate();

        EXPECT_TRUE(mgr.Exists(first));
        EXPECT_TRUE(mgr.Exists(second));
        EXPECT_TRUE(mgr.Exists(third));

        mgr.DestroyImmediate(third);

        EXPECT_TRUE(mgr.Exists(first));
        EXPECT_TRUE(mgr.Exists(second));
        EXPECT_FALSE(mgr.Exists(third));

        mgr.DestroyImmediate(first);

        EXPECT_FALSE(mgr.Exists(first));
        EXPECT_TRUE(mgr.Exists(second));
        EXPECT_FALSE(mgr.Exists(third));

        mgr.DestroyImmediate(second);

        EXPECT_FALSE(mgr.Exists(first));
        EXPECT_FALSE(mgr.Exists(second));
        EXPECT_FALSE(mgr.Exists(third));
    }
}

void TestComponentFlags () {
    ComponentFlags all;
    all.SetFlags<test::FloatA, test::FloatB, test::FloatC>();

    ComponentFlags some;
    some.SetFlags<test::FloatA, test::FloatB>();

    ComponentFlags none;

    EXPECT_TRUE(all.HasAll(all));
    EXPECT_TRUE(all.HasAll(some));
    EXPECT_TRUE(all.HasAll(none));

    EXPECT_FALSE(some.HasAll(all));
    EXPECT_TRUE(some.HasAll(some));
    EXPECT_TRUE(some.HasAll(none));

    EXPECT_FALSE(none.HasAll(all));
    EXPECT_FALSE(none.HasAll(some));
    EXPECT_TRUE(none.HasAll(none));

    EXPECT_TRUE(all.HasAny(all));
    EXPECT_TRUE(all.HasAny(some));
    EXPECT_FALSE(all.HasAny(none));

    EXPECT_TRUE(some.HasAny(all));
    EXPECT_TRUE(some.HasAny(some));
    EXPECT_FALSE(some.HasAny(none));

    EXPECT_FALSE(none.HasAny(all));
    EXPECT_FALSE(none.HasAny(some));
    EXPECT_FALSE(none.HasAny(none));

    EXPECT_FALSE(all.HasNone(all));
    EXPECT_FALSE(all.HasNone(some));
    EXPECT_TRUE(all.HasNone(none));

    EXPECT_FALSE(some.HasNone(all));
    EXPECT_FALSE(some.HasNone(some));
    EXPECT_TRUE(some.HasNone(none));

    EXPECT_TRUE(none.HasNone(all));
    EXPECT_TRUE(none.HasNone(some));
    EXPECT_TRUE(none.HasNone(none));

    ComponentFlags other;
    other.SetFlags<test::FloatC>();

    EXPECT_FALSE(some.HasAll(other));
    EXPECT_FALSE(some.HasAny(other));
    EXPECT_TRUE(some.HasNone(other));

    EXPECT_FALSE(other.HasAll(all));
    EXPECT_TRUE(other.HasAny(all));
    EXPECT_TRUE(other.HasNone(some));
}

void TestFindingComponents () {
    Manager mgr;

    Entity e1 = mgr.CreateEntityImmediate(test::FloatA{ 10 }, test::FloatB{ 100 });
    Entity e2 = mgr.CreateEntityImmediate(test::FloatA{ 20 }, test::FloatB{ 200 });
    Entity e3 = mgr.CreateEntityImmediate(test::FloatA{ 30 });

    EXPECT_TRUE(mgr.HasComponent<test::FloatB>(e1));
    EXPECT_FALSE(mgr.HasComponent<test::TagA>(e1));

    auto e1FloatA = mgr.FindComponent<test::FloatA>(e1);
    EXPECT_TRUE(e1FloatA != nullptr);
    EXPECT_TRUE(e1FloatA && e1FloatA->Value == 10);

    auto e1FloatB = mgr.FindComponent<test::FloatB>(e1);
    EXPECT_TRUE(e1FloatB != nullptr);
    EXPECT_TRUE(e1FloatB && e1FloatB->Value == 100);

    auto e1FloatC = mgr.FindComponent<test::FloatC>(e1);
    EXPECT_FALSE(e1FloatC != nullptr);

    auto e2FloatA = mgr.FindComponent<test::FloatA>(e2);
    EXPECT_TRUE(e2FloatA);
    EXPECT_TRUE(e2FloatA && e2FloatA->Value == 20);

    mgr.DestroyImmediate(e1);

    e1FloatA = mgr.FindComponent<test::FloatA>(e1);
    EXPECT_FALSE(e1FloatA != nullptr);
    EXPECT_FALSE(mgr.HasComponent<test::FloatA>(e1));

    e2FloatA = mgr.FindComponent<test::FloatA>(e2);
    EXPECT_TRUE(e2FloatA && e2FloatA->Value == 20);

    auto e3FloatA = mgr.FindComponent<test::FloatA>(e3);
    EXPECT_TRUE(e3FloatA && e3FloatA->Value == 30);
    auto e3FloatB = mgr.FindComponent<test::FloatB>(e3);
    EXPECT_FALSE(e3FloatB);
}

void TestCompositionChanges () {
    Manager mgr;

    Entity e1 = mgr.CreateEntityImmediate(test::FloatA{ 10 });
    Entity e2 = mgr.CreateEntityImmediate(test::FloatA{ 20 }, test::FloatB{ 200 }, test::FloatC{ 2 });

    {
        mgr.AddComponents(e1, test::FloatB{ 100 });
        auto floatA = mgr.FindComponent<test::FloatA>(e1);
        auto floatB = mgr.FindComponent<test::FloatB>(e1);
        auto floatC = mgr.FindComponent<test::FloatC>(e1);
        EXPECT_TRUE(floatA && floatA->Value == 10);
        EXPECT_TRUE(floatB && floatB->Value == 100);
        EXPECT_FALSE(floatC);
    }

    {
        mgr.RemoveComponents<test::FloatC>(e2);
        auto floatA = mgr.FindComponent<test::FloatA>(e2);
        auto floatB = mgr.FindComponent<test::FloatB>(e2);
        auto floatC = mgr.FindComponent<test::FloatC>(e2);
        EXPECT_TRUE(floatA && floatA->Value == 20);
        EXPECT_TRUE(floatB && floatB->Value == 200);
        EXPECT_FALSE(floatC);
    }

    {
        mgr.AddComponents(e1, test::FloatC{ 1 });
        auto floatA = mgr.FindComponent<test::FloatA>(e1);
        auto floatB = mgr.FindComponent<test::FloatB>(e1);
        auto floatC = mgr.FindComponent<test::FloatC>(e1);
        EXPECT_TRUE(floatA && floatA->Value == 10);
        EXPECT_TRUE(floatB && floatB->Value == 100);
        EXPECT_TRUE(floatC && floatC->Value == 1);
    }
}

struct AddFloatBToFloatA : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);

    void ForEach (Timestep) override {
        A->Value += B->Value;
    }
};

struct AddFloatBToFloatARequireExclude : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_REQUIRE(test::TagA);
    ECS_EXCLUDE(test::TagB);

    void ForEach (Timestep) override {
        A->Value += B->Value;
    }
};

struct AddFloatBToFloatARequireAny : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_REQUIRE_ANY(test::TagA, test::TagB);

    void ForEach (Timestep) override {
        A->Value += B->Value;
    }
};

void TestJob () {
    Manager mgr;

    Entity a = mgr.CreateEntityImmediate(test::FloatA{ 1 }, test::FloatB{ 1 });
    Entity b = mgr.CreateEntityImmediate(test::FloatA{ 2 }, test::FloatB{ 2 }, test::TagA{});
    Entity c = mgr.CreateEntityImmediate(test::FloatA{ 3 }, test::FloatB{ 3 }, test::TagB{});
    Entity d = mgr.CreateEntityImmediate(test::FloatA{ 4 }, test::FloatB{ 4 }, test::TagA{}, test::TagB{});
    Entity e = mgr.CreateEntityImmediate(test::FloatA{ 5 }, test::FloatB{ 5 });

    mgr.RunJob<AddFloatBToFloatA>(0.0f);

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(a)->Value == 2);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(b)->Value == 4);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(c)->Value == 6);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(d)->Value == 8);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(e)->Value == 10);

    mgr.RunJob<AddFloatBToFloatARequireExclude>(0.0f);

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(a)->Value == 2);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(b)->Value == 6);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(c)->Value == 6);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(d)->Value == 8);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(e)->Value == 10);

    mgr.RunJob<AddFloatBToFloatARequireAny>(0.0f);

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(a)->Value == 2);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(b)->Value == 8);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(c)->Value == 9);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(d)->Value == 12);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(e)->Value == 10);
}

struct ReadOtherTestJob : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::EntityReference, Ref);

    ECS_READ_OTHER(test::FloatC, ReadC);

    void ForEach (Timestep) override {
        A->Value = ReadC[Ref->Value]->Value;
    }
};

struct WriteOtherTestJob : public Job {
    ECS_READ(test::FloatB, B);
    ECS_READ(test::EntityReference, Ref);

    ECS_WRITE_OTHER(test::FloatC, WriteC);

    void ForEach (Timestep) override {
        if (HasComponent<test::TagA>(Ref->Value))
            WriteC[Ref->Value]->Value = B->Value;
    }
};

void TestReadWriteOther () {
    Manager mgr;

    Entity target = mgr.CreateEntityImmediate(test::FloatC{ 30.0f }, test::TagA{});
    Entity referencer = mgr.CreateEntityImmediate(test::FloatA{ 10.0f }, test::FloatB{ 20.0f }, test::EntityReference{ target });

    mgr.RunJob<ReadOtherTestJob>(0.0f);

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(referencer)->Value == 30.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatB>(referencer)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatC>(target)->Value = 30.0f);

    mgr.RunJob<WriteOtherTestJob>(0.0f);

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(referencer)->Value == 30.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatB>(referencer)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatC>(target)->Value = 20.0f);

    mgr.RunJob<ReadOtherTestJob>(0.0f);

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(referencer)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatB>(referencer)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatC>(target)->Value = 20.0f);
}

struct SingletonWriteJob : public Job {
    ECS_WRITE_SINGLETON(test::SingletonFloat, Singleton);
    ECS_READ(test::FloatA, A);

    void Run (Timestep dt) override {
        Singleton->Value = 0.0f;
        Job::Run(dt);
    }

    void ForEach (Timestep) override {
        Singleton->Value += A->Value;
    }
};

struct SingletonReadJob : public Job {
    ECS_READ_SINGLETON(test::SingletonFloat, Singleton);
    ECS_READ(test::FloatA, A);

    void ForEach (Timestep) override {
        EXPECT_TRUE(Singleton->Value == 10.0f);
    }
};

void TestSingletonComponents () {
    Manager mgr;

    auto singleton = mgr.GetSingletonComponent<test::SingletonFloat>();
    EXPECT_TRUE(singleton && singleton->Value == 0.0f);

    Entity a = mgr.CreateEntityImmediate(test::FloatA{ 5.0f });
    mgr.CreateEntityImmediate(test::FloatA{ 5.0f });

    mgr.RunJob<SingletonWriteJob>(0.0f);
    EXPECT_TRUE(singleton && singleton->Value == 10.0f);

    mgr.RunJob<SingletonReadJob>(0.0f);

    mgr.FindComponent<test::FloatA>(a)->Value = 10.0f;
    mgr.RunJob<SingletonWriteJob>(0.0f);
    EXPECT_TRUE(singleton && singleton->Value == 15.0f);
}

struct UpdateGroupA : public IUpdateGroup {};
struct UpdateGroupB : IUpdateGroup {};

struct UpdateGroupJobA : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);

    void ForEach (Timestep) override {
        A->Value += B->Value;
    }
};
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(UpdateGroupJobA, UpdateGroupA);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(UpdateGroupJobA, UpdateGroupB);

struct UpdateGroupJobB : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatC, C);

    void ForEach (Timestep) override {
        A->Value += C->Value;
    }
};
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(UpdateGroupJobB, UpdateGroupA);

void TestUpdateGroups () {
    Manager mgr;

    Entity e = mgr.CreateEntityImmediate(test::FloatA{ 1.0f }, test::FloatB{ 2.0f }, test::FloatC{ 3.0f });

    mgr.RunUpdateGroup<UpdateGroupA>(0.0f);

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(e)->Value == 6.0f);

    mgr.RunUpdateGroup<UpdateGroupB>(0.0f);

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(e)->Value == 8.0f);
}

struct UpdateGroupMultiThreading : IUpdateGroup {};

#define MULTI_THREAD_JOB(Type)                                                      \
struct MultiThreadJob##Type : public Job {                                          \
    ECS_READ(test::Type##A, A);                                                     \
    ECS_READ(test::Type##B, B);                                                     \
    ECS_WRITE(test::Type##C, C);                                                    \
                                                                                    \
    void ForEach (Timestep) override {                                              \
        C->Value = std::max(A->Value, B->Value);                                    \
    }                                                                               \
};                                                                                  \
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(MultiThreadJob##Type, UpdateGroupMultiThreading);

#define MULTI_THREAD_SINGLETON_JOB(Type)                                            \
struct MultiThreadSingletonJob##Type : public Job {                                 \
    ECS_READ(test::Type##C, C);                                                     \
    ECS_WRITE_SINGLETON(test::Singleton##Type, Total);                              \
                                                                                    \
    void Run (Timestep dt) override {                                               \
        Total->Value = 0;                                                           \
        Job::Run(dt);                                                               \
    }                                                                               \
                                                                                    \
    void ForEach (Timestep) override {                                              \
        Total->Value += C->Value;                                                   \
    }                                                                               \
};                                                                                  \
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(MultiThreadSingletonJob##Type, UpdateGroupMultiThreading);

MULTI_THREAD_JOB(Double);
MULTI_THREAD_JOB(Float);
MULTI_THREAD_JOB(Int);
MULTI_THREAD_JOB(Uint);

MULTI_THREAD_SINGLETON_JOB(Double);
MULTI_THREAD_SINGLETON_JOB(Float);
MULTI_THREAD_SINGLETON_JOB(Int);
MULTI_THREAD_SINGLETON_JOB(Uint);

#define MULTI_THREAD_ENTITY_COUNT 25000

void InitMultiThreadingTest (Manager * mgr) {
    // Run these first to eliminate the first time init costs
    mgr->RunUpdateGroup<UpdateGroupMultiThreading>(0.0f);
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

enum EThreadingType : uint8_t {
    Single,
    ManualMulti,
    UpdateGroupMulti
};
void ExecuteMultiThreadingTest (Manager * mgr, EThreadingType threading) {
    switch (threading) {
        case EThreadingType::Single: {
            mgr->RunJob<MultiThreadJobDouble>(0.0f);
            mgr->RunJob<MultiThreadJobFloat>(0.0f);
            mgr->RunJob<MultiThreadJobInt>(0.0f);
            mgr->RunJob<MultiThreadJobUint>(0.0f);
            mgr->RunJob<MultiThreadSingletonJobDouble>(0.0f);
            mgr->RunJob<MultiThreadSingletonJobFloat>(0.0f);
            mgr->RunJob<MultiThreadSingletonJobInt>(0.0f);
            mgr->RunJob<MultiThreadSingletonJobUint>(0.0f);
        } break;
        case EThreadingType::UpdateGroupMulti: {
            mgr->RunUpdateGroup<UpdateGroupMultiThreading>(0.0f);
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
    Manager mgr;
    InitMultiThreadingTest(&mgr);
    ExecuteMultiThreadingTest(&mgr, EThreadingType::ManualMulti);
}

void TestMultiThreading () {
    Manager mgr;
    InitMultiThreadingTest(&mgr);
    ExecuteMultiThreadingTest(&mgr, EThreadingType::UpdateGroupMulti);
}

void SpeedTestCalculation (Timestep dt, float & a, const float & b, const float & c) {
    a = std::min(a + c * dt, b);
}

struct SpeedTestJob : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_READ(test::FloatC, C);
    ECS_EXCLUDE(test::TagA);

    void ForEach (Timestep dt) override {
        SpeedTestCalculation(dt, A->Value, B->Value, C->Value);
    }
};

void TestJobSpeed () {
    Manager mgr;

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
        Manager mgr;
        InitMultiThreadingTest(&mgr);

        auto start = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < loopCount; ++i)
            ExecuteMultiThreadingTest(&mgr, EThreadingType::UpdateGroupMulti);
        auto end = std::chrono::high_resolution_clock::now();
        elapsedMulti = end - start;
    }

    std::chrono::duration<double> elapsedManual;
    {
        Manager mgr;
        InitMultiThreadingTest(&mgr);

        auto start = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < loopCount; ++i)
            ExecuteMultiThreadingTest(&mgr, EThreadingType::ManualMulti);
        auto end = std::chrono::high_resolution_clock::now();
        elapsedManual = end - start;
    }

    std::chrono::duration<double> elapsedSingle;
    {
        Manager mgr;
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

void TestCorrectness () {
    TestAssumptions();
    TestEntityComparison();
    TestEntityCreationDestruction();
    TestComponentFlags();
    TestFindingComponents();
    TestCompositionChanges();
    TestJob();
    TestReadWriteOther();
    TestSingletonComponents();
    TestUpdateGroups();
    TestManualMultiThreading();
    TestMultiThreading();
}

void TestSpeed () {
#ifndef _DEBUG
    TestJobSpeed();
    TestMultiThreadingSpeed();
#endif
}

void TestMultipleManagers () {
    const auto threadCount = 4;
    std::future<void> threads[threadCount];

    for (auto i = 0; i < threadCount; ++i)
        threads[i] = std::async(std::launch::async, TestCorrectness);
    for (auto i = 0; i < threadCount; ++i)
        threads[i].wait();
}

// Main
int main () {
    TestCorrectness();
    TestSpeed();
    TestMultipleManagers();

    return s_errorCount;
}
