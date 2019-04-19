#include "../public/ecs.h"

#include <algorithm>
#include <chrono>
#include <iostream>

using namespace ecs;

// Static data
int s_errorCount = 0;

// Test components
namespace test { namespace health {

ECS_TAG_COMPONENT(IsDead);
DEFINE_ECS_COMPONENT(IsDead);

struct Current {
    ECS_COMPONENT();
    float Value = 0;
};
DEFINE_ECS_COMPONENT(Current);

struct Max {
    ECS_COMPONENT();
    float Value = 0;
};
DEFINE_ECS_COMPONENT(Max);

struct Regen {
    ECS_COMPONENT();
    float Value = 0;
};
DEFINE_ECS_COMPONENT(Regen);

}} // namespace test::health

// Helpers
#define EXPECT_TRUE(condition)                                                                  \
    if (!(condition)) {                                                                         \
        ++s_errorCount;                                                                         \
        std::cout << __FUNCTION__ << "(line " << __LINE__ << "): " << #condition << std::endl;  \
    }
#define EXPECT_FALSE(condition) EXPECT_TRUE(!(condition));

// Tests
void TestAssumptions () {
    EXPECT_TRUE(sizeof(Entity) == 8);
    EXPECT_TRUE(std::is_empty<test::health::IsDead>());
    EXPECT_TRUE(sizeof(test::health::Current) == 4);
}

void TestEntityComparison () {
    EXPECT_FALSE((Entity{ 1, 1 }) == (Entity{ 1, 2 }));
    EXPECT_FALSE((Entity{ 1, 1 }) == (Entity{ 2, 1 }));
    EXPECT_TRUE((Entity{ 1, 1 }) == (Entity{ 1, 1 }));
}

void TestEntityInvalid () {
    EXPECT_TRUE(Entity() == Entity::kInvalid);
    EXPECT_FALSE((Entity{ 1, 1 }) == Entity::kInvalid);
}

void TestEntityCreationDestruction () {
    Manager * mgr = new Manager();

    // Create and destroy a single entity
    {
        Entity entity = mgr->CreateEntityImmediate();

        EXPECT_FALSE(entity == Entity::kInvalid);
        EXPECT_TRUE(mgr->Exists(entity));

        mgr->DestroyImmediate(entity);

        EXPECT_FALSE(mgr->Exists(entity));
    }

    // Create several entities and destroy them in an odd order
    {
        Entity first = mgr->CreateEntityImmediate();
        Entity second = mgr->CreateEntityImmediate();
        Entity third = mgr->CreateEntityImmediate();

        EXPECT_TRUE(mgr->Exists(first));
        EXPECT_TRUE(mgr->Exists(second));
        EXPECT_TRUE(mgr->Exists(third));

        mgr->DestroyImmediate(third);

        EXPECT_TRUE(mgr->Exists(first));
        EXPECT_TRUE(mgr->Exists(second));
        EXPECT_FALSE(mgr->Exists(third));

        mgr->DestroyImmediate(first);

        EXPECT_FALSE(mgr->Exists(first));
        EXPECT_TRUE(mgr->Exists(second));
        EXPECT_FALSE(mgr->Exists(third));

        mgr->DestroyImmediate(second);

        EXPECT_FALSE(mgr->Exists(first));
        EXPECT_FALSE(mgr->Exists(second));
        EXPECT_FALSE(mgr->Exists(third));
    }

    delete mgr;
}

void TestComponentFlags () {
    ComponentFlags all;
    all.SetFlags<test::health::Current, test::health::Max, test::health::Regen>();

    ComponentFlags some;
    some.SetFlags<test::health::Current, test::health::Max>();

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
    EXPECT_FALSE(all.HasAny(none)); // Should this be true?

    EXPECT_TRUE(some.HasAny(all));
    EXPECT_TRUE(some.HasAny(some));
    EXPECT_FALSE(some.HasAny(none)); // Should this be true?

    EXPECT_FALSE(none.HasAny(all));
    EXPECT_FALSE(none.HasAny(some));
    EXPECT_FALSE(none.HasAny(none)); // This seems sane-ish

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
    other.SetFlags<test::health::Regen>();

    EXPECT_FALSE(some.HasAll(other));
    EXPECT_FALSE(some.HasAny(other));
    EXPECT_TRUE(some.HasNone(other));

    EXPECT_FALSE(other.HasAll(all));
    EXPECT_TRUE(other.HasAny(all));
    EXPECT_TRUE(other.HasNone(some));
}

void TestFindingComponents () {
    Manager * mgr = new Manager();

    Entity entityA = mgr->CreateEntityImmediate(test::health::Current{ 10 }, test::health::Max{ 100 });
    Entity entityB = mgr->CreateEntityImmediate(test::health::Current{ 20 }, test::health::Max{ 200 });
    Entity entityC = mgr->CreateEntityImmediate(test::health::Current{ 30 });

    EXPECT_TRUE(mgr->HasComponent<test::health::Max>(entityA));
    EXPECT_FALSE(mgr->HasComponent<test::health::IsDead>(entityA));

    auto current = mgr->FindComponent<test::health::Current>(entityA);
    EXPECT_TRUE(current != nullptr);
    EXPECT_TRUE(current && current->Value == 10);

    auto max = mgr->FindComponent<test::health::Max>(entityA);
    EXPECT_TRUE(max != nullptr);
    EXPECT_TRUE(max && max->Value == 100);

    auto regen = mgr->FindComponent<test::health::Regen>(entityA);
    EXPECT_FALSE(regen != nullptr);

    auto currentB = mgr->FindComponent<test::health::Current>(entityB);
    EXPECT_TRUE(currentB);
    EXPECT_TRUE(currentB && currentB->Value == 20);

    mgr->DestroyImmediate(entityA);

    current = mgr->FindComponent<test::health::Current>(entityA);
    EXPECT_FALSE(current != nullptr);
    EXPECT_FALSE(mgr->HasComponent<test::health::Current>(entityA));

    currentB = mgr->FindComponent<test::health::Current>(entityB);
    EXPECT_TRUE(currentB && currentB->Value == 20);

    auto currentC = mgr->FindComponent<test::health::Current>(entityC);
    EXPECT_TRUE(currentC && currentC->Value == 30);
    auto maxC = mgr->FindComponent<test::health::Max>(entityC);
    EXPECT_FALSE(maxC);

    delete mgr;
}

void TestCompositionChanges () {
    Manager * mgr = new Manager();

    Entity a = mgr->CreateEntityImmediate(test::health::Current{ 10 });
    Entity b = mgr->CreateEntityImmediate(test::health::Current{ 20 }, test::health::Max{ 200 }, test::health::Regen{ 2 });

    {
        mgr->AddComponents(a, test::health::Max{ 100 });
        auto cur = mgr->FindComponent<test::health::Current>(a);
        auto max = mgr->FindComponent<test::health::Max>(a);
        auto reg = mgr->FindComponent<test::health::Regen>(a);
        EXPECT_TRUE(cur && cur->Value == 10);
        EXPECT_TRUE(max && max->Value == 100);
        EXPECT_FALSE(reg);
    }

    {
        mgr->RemoveComponents<test::health::Regen>(b);
        auto cur = mgr->FindComponent<test::health::Current>(b);
        auto max = mgr->FindComponent<test::health::Max>(b);
        auto reg = mgr->FindComponent<test::health::Regen>(b);
        EXPECT_TRUE(cur && cur->Value == 20);
        EXPECT_TRUE(max && max->Value == 200);
        EXPECT_FALSE(reg);
    }

    {
        mgr->AddComponents(a, test::health::Regen{ 1 });
        auto cur = mgr->FindComponent<test::health::Current>(a);
        auto max = mgr->FindComponent<test::health::Max>(a);
        auto reg = mgr->FindComponent<test::health::Regen>(a);
        EXPECT_TRUE(cur && cur->Value == 10);
        EXPECT_TRUE(max && max->Value == 100);
        EXPECT_TRUE(reg && reg->Value == 1);
    }

    delete mgr;
}

struct HealthRegen : public Job {
    ECS_WRITE(test::health::Current, Current);
    ECS_READ(test::health::Max, Max);
    ECS_READ(test::health::Regen, Regen);
    ECS_EXCLUDE(test::health::IsDead);

    void ForEach (float dt) override {
        Current->Value = std::min(Current->Value + Regen->Value * dt, Max->Value);
    }
};
REGISTER_ECS_JOB(HealthRegen);

void TestJob () {
    Manager * mgr = new Manager();

    Entity a = mgr->CreateEntityImmediate(test::health::Current{ 10 }, test::health::Max{ 100 }, test::health::Regen{ 1 });
    Entity b = mgr->CreateEntityImmediate(test::health::Current{ 20 }, test::health::Max{ 200 }, test::health::Regen{ 2 });
    Entity c = mgr->CreateEntityImmediate(test::health::Current{ 30 }, test::health::Max{ 300 }, test::health::Regen{ 3 });
    Entity d = mgr->CreateEntityImmediate(test::health::Current{ 40 }, test::health::Max{ 400 }, test::health::Regen{ 4 }, test::health::IsDead{});

    mgr->Update(10);

    test::health::Current * healthA = mgr->FindComponent<test::health::Current>(a);
    test::health::Current * healthB = mgr->FindComponent<test::health::Current>(b);
    test::health::Current * healthC = mgr->FindComponent<test::health::Current>(c);
    test::health::Current * healthD = mgr->FindComponent<test::health::Current>(d);
    EXPECT_TRUE(healthA && healthA->Value == 20);
    EXPECT_TRUE(healthB && healthB->Value == 40);
    EXPECT_TRUE(healthC && healthC->Value == 60);
    EXPECT_TRUE(healthD && healthD->Value == 40);

    mgr->Update(1000);

    healthA = mgr->FindComponent<test::health::Current>(a);
    healthB = mgr->FindComponent<test::health::Current>(b);
    healthC = mgr->FindComponent<test::health::Current>(c);
    healthD = mgr->FindComponent<test::health::Current>(d);
    EXPECT_TRUE(healthA && healthA->Value == 100);
    EXPECT_TRUE(healthB && healthB->Value == 200);
    EXPECT_TRUE(healthC && healthC->Value == 300);
    EXPECT_TRUE(healthD && healthD->Value == 40);

    delete mgr;
}

void TestJobSpeed () {
    Manager * mgr = new Manager();

    uint32_t entityCount = 100000;
    uint32_t loopCount = 1 * 60 * 60;

    for (uint32_t i = 0; i < entityCount; ++i)
        mgr->CreateEntityImmediate(test::health::Current{ 0 }, test::health::Max{ 1000 }, test::health::Regen{ 1.0f });

    auto start = std::chrono::system_clock::now();
    for (uint32_t i = 0; i < loopCount; ++i)
        mgr->Update(1 / 60.0f);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedJob = end - start;

    uint64_t benchmarkLoops = entityCount * (uint64_t)loopCount;
    float cur = 0;
    float max = 1000;
    float regen = 1;
    start = std::chrono::system_clock::now();
    for (uint64_t i = 0; i < benchmarkLoops; ++i)
        cur = std::min(cur + regen * (1/60.0f), max);
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedBenchmark = end - start;

    double maxRatio = 2.0;
    EXPECT_FALSE(elapsedJob.count() > elapsedBenchmark.count() * maxRatio);
    if (elapsedJob.count() > elapsedBenchmark.count() * maxRatio)
        std::cout << "  " << elapsedJob.count() * 1000 << "ms vs " << elapsedBenchmark.count() * 1000 << "ms (" << 100 * elapsedJob.count() / elapsedBenchmark.count() << "%)" << std::endl;

    delete mgr;
}

// Main
int main () {
    TestAssumptions();
    TestEntityComparison();
    TestEntityInvalid();
    TestEntityCreationDestruction();
    TestComponentFlags();
    TestFindingComponents();
    TestCompositionChanges();
    TestJob();

#ifndef _DEBUG
    TestJobSpeed();
#endif

    return s_errorCount;
}
