#include "../ecs/ecs.h"

#include <algorithm>
#include <chrono>
#include <iostream>

using namespace ecs;

// Static data
int s_errorCount = 0;

// Test components
namespace test {

struct TagA {};
struct TagB {};
struct TagC {};

struct EntityReference { Entity Value; };

struct FloatA { float Value = 0.0f; };
struct FloatB { float Value = 0.0f; };
struct FloatC { float Value = 0.0f; };

struct SingletonA : ISingletonComponent { float Value = 0.0f; };

}

// Helpers
#define EXPECT_TRUE(condition)                                                                  \
    if (!(condition)) {                                                                         \
        ++s_errorCount;                                                                         \
        std::cout << __FUNCTION__ << "(line " << __LINE__ << "): " << #condition << std::endl;  \
    }
#define EXPECT_FALSE(condition) EXPECT_TRUE(!(condition));

// Tests
void TestAssumptions () {
    EXPECT_TRUE(sizeof(Entity) == (sizeof(uint32_t) + sizeof(uint32_t)));
    EXPECT_TRUE(std::is_empty<test::TagA>());
    EXPECT_TRUE(sizeof(test::FloatA) == sizeof(float));
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

    void ForEach (float dt) override {
        A->Value += B->Value;
    }
};

struct AddFloatBToFloatARequireExclude : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_REQUIRE(test::TagA);
    ECS_EXCLUDE(test::TagB);

    void ForEach (float dt) override {
        A->Value += B->Value;
    }
};

struct AddFloatBToFloatARequireAny : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_REQUIRE_ANY(test::TagA, test::TagB);

    void ForEach (float dt) override {
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

    void ForEach (float dt) override {
        A->Value = ReadC[Ref->Value]->Value;
    }
};

struct WriteOtherTestJob : public Job {
    ECS_READ(test::FloatB, B);
    ECS_READ(test::EntityReference, Ref);

    ECS_WRITE_OTHER(test::FloatC, WriteC);

    void ForEach (float dt) override {
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
    ECS_WRITE_SINGLETON(test::SingletonA, Singleton);
    ECS_READ(test::FloatA, A);

    void Run (float dt) override {
        Singleton->Value = 0.0f;
        Job::Run(dt);
    }

    void ForEach (float dt) override {
        Singleton->Value += A->Value;
    }
};

struct SingletonReadJob : public Job {
    ECS_READ_SINGLETON(test::SingletonA, Singleton);
    ECS_READ(test::FloatA, A);

    void ForEach (float dt) override {
        EXPECT_TRUE(Singleton->Value == 10.0f);
    }
};

void TestSingletonComponents () {
    Manager mgr;

    auto singleton = mgr.GetSingletonComponent<test::SingletonA>();
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

void SpeedTestCalculation (float dt, float & a, const float & b, const float & c) {
    a = std::min(a + c * dt, b);
}

struct SpeedTestJob : public Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_READ(test::FloatC, C);
    ECS_EXCLUDE(test::TagA);

    void ForEach (float dt) override {
        SpeedTestCalculation(dt, A->Value, B->Value, C->Value);
    }
};

void TestJobSpeed () {
    Manager mgr;

    uint32_t entityCount = 100000;
    uint32_t loopCount = 1 * 60 * 60;

    for (uint32_t i = 0; i < entityCount; ++i)
        mgr.CreateEntityImmediate(test::FloatA{ 0 }, test::FloatB{ 1000 }, test::FloatC{ 1.0f });

    float timestep = 1 / 60.0f;
    auto start = std::chrono::system_clock::now();
    for (uint32_t i = 0; i < loopCount; ++i)
        mgr.RunJob<SpeedTestJob>(timestep);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedJob = end - start;

    uint64_t benchmarkLoops = entityCount * (uint64_t)loopCount;
    float a = 0;
    float b = 1000;
    float c = 1;
    start = std::chrono::system_clock::now();
    for (uint64_t i = 0; i < benchmarkLoops; ++i)
        SpeedTestCalculation(timestep, a, b, c);
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedBenchmark = end - start;

    double maxRatio = 2.0;
    EXPECT_FALSE(elapsedJob.count() > elapsedBenchmark.count() * maxRatio);
    if (elapsedJob.count() > elapsedBenchmark.count() * maxRatio)
        std::cout << "  " << elapsedJob.count() * 1000 << "ms vs " << elapsedBenchmark.count() * 1000 << "ms (" << 100 * elapsedJob.count() / elapsedBenchmark.count() << "%)" << std::endl;
}

// Main
int main () {
    TestAssumptions();
    TestEntityComparison();
    TestEntityCreationDestruction();
    TestComponentFlags();
    TestFindingComponents();
    TestCompositionChanges();
    TestJob();
    TestReadWriteOther();
    TestSingletonComponents();

#ifndef _DEBUG
    TestJobSpeed();
#endif

    return s_errorCount;
}
