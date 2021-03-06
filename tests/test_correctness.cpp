// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#include "tests.h"
#include "test_components.h"
#include "test_correctness.h"
#include "test_multi_threading.h"

namespace test {

void TestAssumptions () {
    static_assert(sizeof(ecs::Entity) == (sizeof(uint32_t) + sizeof(uint32_t)), "Unexpected Entity size");
    static_assert(std::is_empty<test::TagA>(), "Unexpected tag size");
    static_assert(sizeof(test::FloatA) == sizeof(float), "Unexpected struct size");
}

void TestEntityComparison () {
    EXPECT_FALSE((ecs::Entity{ 1, 1 }) == (ecs::Entity{ 1, 2 }));
    EXPECT_FALSE((ecs::Entity{ 1, 1 }) == (ecs::Entity{ 2, 1 }));
    EXPECT_TRUE((ecs::Entity{ 1, 1 }) == (ecs::Entity{ 1, 1 }));
    EXPECT_FALSE((ecs::Entity{ 2, 2 }) != (ecs::Entity{ 2, 2 }));
    EXPECT_TRUE((ecs::Entity{ 2, 1 }) != (ecs::Entity{ 2, 2 }));

    ecs::EntityId id11 = ecs::Entity{ 1, 1 }.GetId();
    ecs::EntityId id12 = ecs::Entity{ 1, 2 }.GetId();
    ecs::EntityId id21 = ecs::Entity{ 2, 1 }.GetId();
    EXPECT_TRUE(ecs::Entity::FromId(id11) == (ecs::Entity{ 1, 1 }));
    EXPECT_FALSE(id12 == id21);
    EXPECT_TRUE(ecs::Entity::FromId(id21) == (ecs::Entity{ 2, 1 }));

    std::hash<ecs::Entity> entityHasher;
    std::hash<ecs::EntityId> idHasher;
    EXPECT_TRUE(entityHasher(ecs::Entity{ 2, 1 }) == idHasher(id21));
    EXPECT_FALSE(entityHasher(ecs::Entity{ 1, 2 }) == entityHasher(ecs::Entity{ 2, 1 }));
}

void TestEntityCreationDestruction () {
    ecs::Manager mgr;

    // Create and destroy a single entity
    {
        ecs::Entity entity = mgr.CreateEntityImmediate();

        EXPECT_TRUE(mgr.Exists(entity));

        mgr.DestroyImmediate(entity);

        EXPECT_FALSE(mgr.Exists(entity));
    }

    // Create several entities and destroy them in an odd order
    {
        ecs::Entity first = mgr.CreateEntityImmediate();
        ecs::Entity second = mgr.CreateEntityImmediate();
        ecs::Entity third = mgr.CreateEntityImmediate();

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
    ecs::impl::ComponentFlags all;
    all.SetFlags<test::FloatA, test::FloatB, test::FloatC>();

    ecs::impl::ComponentFlags some;
    some.SetFlags<test::FloatA, test::FloatB>();

    ecs::impl::ComponentFlags none;

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

    ecs::impl::ComponentFlags other;
    other.SetFlags<test::FloatC>();

    EXPECT_FALSE(some.HasAll(other));
    EXPECT_FALSE(some.HasAny(other));
    EXPECT_TRUE(some.HasNone(other));

    EXPECT_FALSE(other.HasAll(all));
    EXPECT_TRUE(other.HasAny(all));
    EXPECT_TRUE(other.HasNone(some));
}

void TestComposition () {
    ecs::impl::Composition compA1;
    compA1.SetComponents(FloatA{ 1.0f });

    EXPECT_TRUE(compA1.GetComponentFlags().Has<FloatA>());

    ecs::impl::Composition compA2;
    compA2.SetComponents(FloatA{ 2.0f }, FloatB{ 4.0f });

    EXPECT_FALSE(compA1.GetComponentFlags() == compA2.GetComponentFlags());
    EXPECT_FALSE(compA1.GetHash() == compA2.GetHash());
    EXPECT_FALSE(compA1 == compA2);

    ecs::impl::Composition compA1Dupe;
    compA1Dupe.SetComponents(FloatA{ 11.0f });

    ecs::impl::Composition compA2Dupe;
    compA2Dupe.SetComponents(FloatA{ 22.0f }, FloatB{ 44.0f });

    EXPECT_TRUE(compA1.GetHash() == compA1Dupe.GetHash());
    EXPECT_TRUE(compA1 == compA1Dupe);
    EXPECT_TRUE(compA2.GetHash() == compA2Dupe.GetHash());
    EXPECT_TRUE(compA2 == compA2Dupe);

    ecs::impl::Composition compWithFlag;
    compWithFlag.SetComponents(FloatA{ 3.0f }, FloatB{ 6.0f }, TagA{}, TagB{});
    const ecs::impl::ComponentCollectionFactory& factory = compWithFlag.GetComponentCollectionFactory();

    EXPECT_FALSE(factory.find(ecs::impl::GetComponentId<FloatA>()) == factory.end());
    EXPECT_FALSE(factory.find(ecs::impl::GetComponentId<FloatB>()) == factory.end());
    EXPECT_TRUE(factory.find(ecs::impl::GetComponentId<TagA>()) == factory.end());
    EXPECT_TRUE(factory.find(ecs::impl::GetComponentId<TagB>()) == factory.end());
}

void TestFindingComponents () {
    ecs::Manager mgr;

    ecs::Entity e1 = mgr.CreateEntityImmediate(test::FloatA{ 10 }, test::FloatB{ 100 });
    ecs::Entity e2 = mgr.CreateEntityImmediate(test::FloatA{ 20 }, test::FloatB{ 200 });
    ecs::Entity e3 = mgr.CreateEntityImmediate(test::FloatA{ 30 });

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
    ecs::Manager mgr;

    ecs::Entity e1 = mgr.CreateEntityImmediate(test::FloatA{ 10 });

    ecs::Entity firstInChunk = mgr.CreateEntityImmediate(test::FloatA{ 100 }, test::FloatB{ 1000 }, test::FloatC{ 10 });
    ecs::Entity e2 = mgr.CreateEntityImmediate(test::FloatA{ 20 }, test::FloatB{ 200 }, test::FloatC{ 2 });
    ecs::Entity thirdInChunk = mgr.CreateEntityImmediate(test::FloatA{ 200 }, test::FloatB{ 2000 }, test::FloatC{ 20 });

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

    // Make sure that other entities weren't affected
    {
        auto floatA = mgr.FindComponent<test::FloatA>(firstInChunk);
        auto floatB = mgr.FindComponent<test::FloatB>(firstInChunk);
        auto floatC = mgr.FindComponent<test::FloatC>(firstInChunk);
        EXPECT_TRUE(floatA && floatA->Value == 100);
        EXPECT_TRUE(floatB && floatB->Value == 1000);
        EXPECT_TRUE(floatC && floatC->Value == 10);
    }

    {
        auto floatA = mgr.FindComponent<test::FloatA>(thirdInChunk);
        auto floatB = mgr.FindComponent<test::FloatB>(thirdInChunk);
        auto floatC = mgr.FindComponent<test::FloatC>(thirdInChunk);
        EXPECT_TRUE(floatA && floatA->Value == 200);
        EXPECT_TRUE(floatB && floatB->Value == 2000);
        EXPECT_TRUE(floatC && floatC->Value == 20);
    }
}

void TestDestroyMiddleOfChunk () {
    ecs::Manager mgr;

    ecs::Entity firstInChunk = mgr.CreateEntityImmediate(test::FloatA{ 100 }, test::FloatB{ 1000 }, test::FloatC{ 10 });
    ecs::Entity middleInChunk = mgr.CreateEntityImmediate(test::FloatA{ 20 }, test::FloatB{ 200 }, test::FloatC{ 2 });
    ecs::Entity thirdInChunk = mgr.CreateEntityImmediate(test::FloatA{ 200 }, test::FloatB{ 2000 }, test::FloatC{ 20 });

    mgr.DestroyImmediate(middleInChunk);

    // Make sure that other entities weren't affected
    {
        auto floatA = mgr.FindComponent<test::FloatA>(firstInChunk);
        auto floatB = mgr.FindComponent<test::FloatB>(firstInChunk);
        auto floatC = mgr.FindComponent<test::FloatC>(firstInChunk);
        EXPECT_TRUE(floatA && floatA->Value == 100);
        EXPECT_TRUE(floatB && floatB->Value == 1000);
        EXPECT_TRUE(floatC && floatC->Value == 10);
    }

    {
        auto floatA = mgr.FindComponent<test::FloatA>(thirdInChunk);
        auto floatB = mgr.FindComponent<test::FloatB>(thirdInChunk);
        auto floatC = mgr.FindComponent<test::FloatC>(thirdInChunk);
        EXPECT_TRUE(floatA && floatA->Value == 200);
        EXPECT_TRUE(floatB && floatB->Value == 2000);
        EXPECT_TRUE(floatC && floatC->Value == 20);
    }
}

struct AddFloatBToFloatA : ecs::Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);

    void ForEach () override {
        A->Value += B->Value;
    }
};

struct AddFloatBToFloatARequireExclude : ecs::Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_REQUIRE(test::TagA);
    ECS_EXCLUDE(test::TagB);

    void ForEach () override {
        A->Value += B->Value;
    }
};

struct AddFloatBToFloatARequireAny : ecs::Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);
    ECS_REQUIRE_ANY(test::TagA, test::TagB);

    void ForEach () override {
        A->Value += B->Value;
    }
};

void TestJob () {
    ecs::Manager mgr;

    ecs::Entity a = mgr.CreateEntityImmediate(test::FloatA{ 1 }, test::FloatB{ 1 });
    ecs::Entity b = mgr.CreateEntityImmediate(test::FloatA{ 2 }, test::FloatB{ 2 }, test::TagA{});
    ecs::Entity c = mgr.CreateEntityImmediate(test::FloatA{ 3 }, test::FloatB{ 3 }, test::TagB{});
    ecs::Entity d = mgr.CreateEntityImmediate(test::FloatA{ 4 }, test::FloatB{ 4 }, test::TagA{}, test::TagB{});
    ecs::Entity e = mgr.CreateEntityImmediate(test::FloatA{ 5 }, test::FloatB{ 5 });

    mgr.RunJob<AddFloatBToFloatA>();

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(a)->Value == 2);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(b)->Value == 4);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(c)->Value == 6);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(d)->Value == 8);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(e)->Value == 10);

    mgr.RunJob<AddFloatBToFloatARequireExclude>();

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(a)->Value == 2);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(b)->Value == 6);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(c)->Value == 6);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(d)->Value == 8);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(e)->Value == 10);

    mgr.RunJob<AddFloatBToFloatARequireAny>();

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(a)->Value == 2);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(b)->Value == 8);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(c)->Value == 9);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(d)->Value == 12);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(e)->Value == 10);
}

struct ReadOtherTestJob : ecs::Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::EntityReference, Ref);

    ECS_READ_OTHER(test::FloatC, ReadC);

    void ForEach () override {
        A->Value = ReadC.Find(Ref->Value)->Value;
    }
};

struct WriteOtherTestJob : ecs::Job {
    ECS_READ(test::FloatB, B);
    ECS_READ(test::EntityReference, Ref);

    ECS_WRITE_OTHER(test::FloatC, WriteC);

    void ForEach () override {
        if (HasComponent<test::TagA>(Ref->Value))
            WriteC.Find(Ref->Value)->Value = B->Value;
    }
};

void TestReadWriteOther () {
    ecs::Manager mgr;

    ecs::Entity target = mgr.CreateEntityImmediate(test::FloatC{ 30.0f }, test::TagA{});
    ecs::Entity referencer = mgr.CreateEntityImmediate(test::FloatA{ 10.0f }, test::FloatB{ 20.0f }, test::EntityReference{ target });

    mgr.RunJob<ReadOtherTestJob>();

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(referencer)->Value == 30.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatB>(referencer)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatC>(target)->Value = 30.0f);

    mgr.RunJob<WriteOtherTestJob>();

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(referencer)->Value == 30.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatB>(referencer)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatC>(target)->Value = 20.0f);

    mgr.RunJob<ReadOtherTestJob>();

    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(referencer)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatB>(referencer)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatC>(target)->Value = 20.0f);
}

struct SingletonWriteJob : ecs::Job {
    ECS_WRITE_SINGLETON(test::SingletonFloat, Singleton);
    ECS_READ(test::FloatA, A);

    void Run () override {
        Singleton->Value = 0.0f;
        ecs::Job::Run();
    }

    void ForEach () override {
        Singleton->Value += A->Value;

        // Mostly just to make sure this compiles
        test::SingletonFloat& singleton = *Singleton;
        singleton.Value = Singleton->Value;
    }
};

struct SingletonReadJob : ecs::Job {
    ECS_READ_SINGLETON(test::SingletonFloat, Singleton);
    ECS_READ(test::FloatA, A);

    void ForEach () override {
        EXPECT_TRUE(Singleton->Value == 10.0f);

        const test::SingletonFloat& singleton = *Singleton;
        EXPECT_TRUE(singleton.Value == 10.0f);
    }
};

void TestSingletonComponents () {
    ecs::Manager mgr;

    auto singleton = mgr.GetSingletonComponent<test::SingletonFloat>();
    EXPECT_TRUE(singleton && singleton->Value == 0.0f);

    ecs::Entity a = mgr.CreateEntityImmediate(test::FloatA{ 5.0f });
    mgr.CreateEntityImmediate(test::FloatA{ 5.0f });

    mgr.RunJob<SingletonWriteJob>();
    EXPECT_TRUE(singleton && singleton->Value == 10.0f);

    mgr.RunJob<SingletonReadJob>();

    mgr.FindComponent<test::FloatA>(a)->Value = 10.0f;
    mgr.RunJob<SingletonWriteJob>();
    EXPECT_TRUE(singleton && singleton->Value == 15.0f);
}

struct ChunkJobExecute : ecs::Job {
    ECS_WRITE(test::FloatA, A);
    ECS_READ(test::FloatB, B);

    void ForEachChunk () override {
        auto aArray = A.GetChunkComponentArray();
        auto bArray = B.GetChunkComponentArray();
        for (uint32_t i = 0; i < GetChunkEntityCount(); ++i)
            aArray[i].Value += bArray[i].Value;
    }
};

struct ChunkJobValidate : ecs::Job {
    ECS_READ(test::FloatA, A);

    void ForEach () override {
        EXPECT_TRUE(A->Value == 3.0f);
    }
};

void TestChunkJob () {
    ecs::Manager mgr;

    mgr.CreateEntityImmediate(test::FloatA{ 1.0f }, test::FloatB{ 2.0f });
    mgr.CreateEntityImmediate(test::FloatA{ 1.0f }, test::FloatB{ 2.0f });
    mgr.CreateEntityImmediate(test::FloatA{ 1.0f }, test::FloatB{ 2.0f });
    mgr.CreateEntityImmediate(test::FloatA{ 1.0f }, test::FloatB{ 2.0f }, test::FloatC{ 3.0f });
    mgr.CreateEntityImmediate(test::FloatA{ 1.0f }, test::FloatB{ 2.0f }, test::FloatC{ 3.0f });
    mgr.CreateEntityImmediate(test::FloatA{ 3.0f });

    mgr.RunJob<ChunkJobExecute>();
    mgr.RunJob<ChunkJobValidate>();
}

struct QueuedChangeJob : ecs::Job {
    ECS_READ(ecs::Entity, Ent);
    ECS_READ(test::EntityReference, Ref);
    ECS_REQUIRE(test::TagA);

    void ForEach () override {
        QueueAddComponents(*Ent, test::FloatA{ 1.0f });
        QueueRemoveComponents<test::TagA>(*Ent);
        QueueCreateEntity(test::FloatA{ 2.0f });
        QueueDestroyEntity(Ref->Value);
    }
};

struct QueuedChangeTotal: ecs::Job {
    ECS_READ(test::FloatA, A);

    ECS_WRITE_SINGLETON(test::SingletonFloat, Total);

    void ForEach () override {
        Total->Value += A->Value;
    }
};

void TestQueuedChanges () {
    ecs::Manager mgr;

    ecs::Entity a = mgr.CreateEntityImmediate(test::FloatA{ 10.0f });
    ecs::Entity b = mgr.CreateEntityImmediate(test::FloatA{ 20.0f });
    ecs::Entity c = mgr.CreateEntityImmediate(test::TagA{}, test::EntityReference{ a });
    ecs::Entity d = mgr.CreateEntityImmediate(test::TagA{}, test::EntityReference{ b });

    mgr.RunJob<QueuedChangeJob>();
    mgr.RunJob<QueuedChangeTotal>();

    EXPECT_FALSE(mgr.Exists(a));
    EXPECT_FALSE(mgr.Exists(b));
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(c)->Value == 1.0f);
    EXPECT_TRUE(mgr.FindComponent<test::FloatA>(d)->Value == 1.0f);
    EXPECT_TRUE(mgr.GetSingletonComponent<test::SingletonFloat>()->Value == 6.0f);
}

struct CloneJob : ecs::Job {
    ECS_READ(ecs::Entity, Ent);
    ECS_REQUIRE(FloatA, FloatB, FloatC);

    ECS_WRITE_SINGLETON(SingletonUint, Count);

    void Run () override {
        Count->Value = 0;
        Job::Run();
    }

    void ForEach () override {
        QueueCloneEntity(*Ent);
        Count->Value++;
    }
};

struct CloneValidateJob : ecs::Job {
    ECS_READ(ecs::Entity, Ent);
    ECS_READ(test::EntityReference, Ref);

    void ForEach () override {
        EXPECT_TRUE(*Ent == Ref->Value);
    }
};

void TestEntityCloning () {
    ecs::Manager mgr;

    ecs::Entity og = mgr.CreateEntityImmediate(FloatA{ 1.0f }, FloatB{ 2.0f }, FloatC{ 3.0f });
    ecs::Entity clone = mgr.Clone(og);

    EXPECT_FALSE(og == clone);
    mgr.AddComponents(og, EntityReference{ og });
    mgr.AddComponents(clone, EntityReference{ clone });
    mgr.RunJob<CloneValidateJob>();

    EXPECT_TRUE(mgr.FindComponent<FloatA>(clone)->Value == 1.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatB>(clone)->Value == 2.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatC>(clone)->Value == 3.0f);

    mgr.AddComponents(clone, FloatA{ 10.0f }, FloatB{ 20.0f }, FloatC{ 30.0f });

    EXPECT_TRUE(mgr.FindComponent<FloatA>(og)->Value == 1.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatB>(og)->Value == 2.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatC>(og)->Value == 3.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatA>(clone)->Value == 10.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatB>(clone)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatC>(clone)->Value == 30.0f);

    mgr.RunJob<CloneJob>();
    EXPECT_TRUE(mgr.GetSingletonComponent<SingletonUint>()->Value == 2);

    mgr.RunJob<CloneJob>();
    EXPECT_TRUE(mgr.GetSingletonComponent<SingletonUint>()->Value == 4);
}

struct PrefabToSpawn : ecs::ISingletonComponent { ECS_COMPONENT(PrefabToSpawn) ecs::Prefab Value; };

struct PrefabJob : ecs::Job {
    ECS_REQUIRE(FloatA, FloatB, FloatC);

    ECS_READ_SINGLETON(PrefabToSpawn, Prefab);
    ECS_WRITE_SINGLETON(SingletonUint, Count);

    void Run () override {
        Count->Value = 0;
        Job::Run();
    }

    void ForEach () override {
        QueueSpawnPrefab(Prefab->Value);
        Count->Value++;
    }
};

void TestPrefabs () {
    ecs::Manager mgr;

    ecs::Prefab prefab = mgr.CreatePrefab(FloatA{ 1.0f }, FloatB{ 2.0f }, FloatC{ 3.0f });
    ecs::Entity spawned = mgr.SpawnPrefab(prefab);

    EXPECT_TRUE(mgr.FindComponent<FloatA>(spawned)->Value == 1.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatB>(spawned)->Value == 2.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatC>(spawned)->Value == 3.0f);

    mgr.AddComponents(spawned, FloatA{ 10.0f }, FloatB{ 20.0f }, FloatC{ 30.0f });

    EXPECT_TRUE(mgr.FindComponent<FloatA>(spawned)->Value == 10.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatB>(spawned)->Value == 20.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatC>(spawned)->Value == 30.0f);

    ecs::Entity spawned2 = mgr.SpawnPrefab(prefab);

    EXPECT_TRUE(mgr.FindComponent<FloatA>(spawned2)->Value == 1.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatB>(spawned2)->Value == 2.0f);
    EXPECT_TRUE(mgr.FindComponent<FloatC>(spawned2)->Value == 3.0f);

    mgr.GetSingletonComponent<PrefabToSpawn>()->Value = prefab;

    mgr.RunJob<PrefabJob>();
    EXPECT_TRUE(mgr.GetSingletonComponent<SingletonUint>()->Value == 2);

    mgr.RunJob<PrefabJob>();
    EXPECT_TRUE(mgr.GetSingletonComponent<SingletonUint>()->Value == 4);
}

struct DynamicMemoryComponent {
    ECS_COMPONENT(DynamicMemoryComponent)

    std::vector<int> IntVector;
};

void TestDynamicMemoryComponent () {
    ecs::Manager ecs;

    ecs::Entity e1 = ecs.CreateEntityImmediate(DynamicMemoryComponent());
    ecs::Entity e2 = ecs.CreateEntityImmediate(DynamicMemoryComponent(), FloatA{ 100.0f });

    DynamicMemoryComponent* dmc1 = ecs.FindComponent<DynamicMemoryComponent>(e1);
    dmc1->IntVector.push_back(1);
    dmc1->IntVector.push_back(2);
    dmc1->IntVector.push_back(3);

    DynamicMemoryComponent* dmc2 = ecs.FindComponent<DynamicMemoryComponent>(e2);
    dmc2->IntVector.push_back(10);
    dmc2->IntVector.push_back(20);

    // Move to the same chunk as e2
    ecs.AddComponents(e1, FloatA{ 10.0f });

    dmc1 = ecs.FindComponent<DynamicMemoryComponent>(e1);

    EXPECT_TRUE(dmc1->IntVector.size() == 3);
    EXPECT_TRUE(dmc1->IntVector[0] == 1);
    EXPECT_TRUE(dmc1->IntVector[1] == 2);
    EXPECT_TRUE(dmc1->IntVector[2] == 3);

    dmc2 = ecs.FindComponent<DynamicMemoryComponent>(e2);

    EXPECT_TRUE(dmc2->IntVector.size() == 2);
    EXPECT_TRUE(dmc2->IntVector[0] == 10);
    EXPECT_TRUE(dmc2->IntVector[1] == 20);

    // Move e2 back to the chunk where e1 was
    ecs.RemoveComponents<FloatA>(e2);

    dmc1 = ecs.FindComponent<DynamicMemoryComponent>(e1);

    EXPECT_TRUE(dmc1->IntVector.size() == 3);
    EXPECT_TRUE(dmc1->IntVector[0] == 1);
    EXPECT_TRUE(dmc1->IntVector[1] == 2);
    EXPECT_TRUE(dmc1->IntVector[2] == 3);

    dmc2 = ecs.FindComponent<DynamicMemoryComponent>(e2);

    EXPECT_TRUE(dmc2->IntVector.size() == 2);
    EXPECT_TRUE(dmc2->IntVector[0] == 10);
    EXPECT_TRUE(dmc2->IntVector[1] == 20);
}

void TestCorrectness () {
    TestAssumptions();
    TestEntityComparison();
    TestEntityCreationDestruction();
    TestComponentFlags();
    TestComposition();
    TestFindingComponents();
    TestCompositionChanges();
    TestDestroyMiddleOfChunk();
    TestJob();
    TestReadWriteOther();
    TestSingletonComponents();
    TestChunkJob();
    TestManualMultiThreading();
    TestQueuedChanges();
    TestEntityCloning();
    TestPrefabs();
    TestDynamicMemoryComponent();
}

}
