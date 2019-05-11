# Ecs
Entity Component System - A data-oriented solution for storing and processing state

### Usage Example
```C++
#include "ecs/ecs.h"

namespace health {
struct Current { float Value; };
struct Regen { float Value; };
}

struct RegenJob : ecs::Job {
    ECS_WRITE(health::Current, Current);
    ECS_READ(health::Regen, Regen);

    void ForEach (ecs::Timestep dt) override {
        Current->Value += Regen->Value * dt;
    }
};

int main () {
    ecs::Manager mgr;
    ecs::Entity a = mgr.CreateEntityImmediate(health::Current{10}, health::Regen{1});

    mgr.RunJob<RegenJob>(1.0f /* dt */);

    std::cout << mgr.FindComponent<health::Current>(a)->Value;
    /* 11 */
}
```

# Current Features
### Creation/Destruction of Entities
```C++
ecs::Entity entity = mgr.CreateEntityImmediate(ComponentA{10.0f}, ComponentB{20.0f}, ...);
ecs::Entity clone = mgr.Clone(entity);
mgr.DestroyEntityImmediate(entity);
```

### Component Manipulation
Note: Standard components must be plain old data
```C++
mgr.AddComponents(entity, ComponentC{30.0f}, ComponentD{1});
mgr.RemoveComponents<ComponentA, ComponentB>(entity);
mgr.FindComponent<ComponentC>(entity)->Value = 5.0f;
mgr.HasComponent<ComponentD>(entity) == true;
```

### Singleton Components
```C++
struct CameraPosition : ecs::ISingletonComponent { float3 Value; };

mgr.GetSingletonComponent<CameraPosition>()->Value = float3(1.0f, 1.0f, 1.0f);
```

### Jobs
```C++
struct ExampleJob : public ecs::Job {
    // ForEach will run once per entity that satisfies
    // all of these conditions
    ECS_WRITE(ComponentA, A);
    ECS_READ(ComponentB, B);
    ECS_REQUIRE(ComponentC, ComponentD);
    ECS_EXCLUDE(ComponentE, ComponentF);
    ECS_REQUIRE_ANY(ComponentG, ComponentH);

    // These are random accessors for components on other entites
    ECS_READ_OTHER(ComponentI, I);
    ECS_WRITE_OTHER(ComponentJ, J);

    // Singleton components are guaranteed to have one instance per Manager
    ECS_READ_SINGLETON(SingletonK, K);
    ECS_WRITE_SINGLETON(SingletonL, L);

    // These can explicitly control the order jobs within an update group
    // Use sparingly, as data depedencies can sort most jobs automatically
    ECS_RUN_THIS_AFTER(PrereqJob);
    ECS_RUN_THIS_BEFORE(DependentJob);

    int m_internalData = 0;

    void Run (ecs::Timestep dt) override {
        m_internalData = K->Value;

        // Executes the ForEach on each entity that matches all
        // WRITE/READ/REQUIRE/EXCLUDE/REQUIRE_ANY filters
        ecs::Job::Run(dt);

        L->Value = m_internalData;
    }

    void ForEach (ecs::Timestep dt) override {
        if (ComponentI * i = I.Find(B->Value))
            A->Value += i->Value * dt;
        if (ComponentJ * j = J.Find(B->Value))
            j->Value = A->Value;
        m_internalData += A->Value;
    }
};

int main () {
    Manager mgr;
    mgr->RunJob<ExampleJob>(0.0f /* dt */);
}
```

### Update groups
Jobs within update groups are automatically run multi-threaded.
See Jobs section for explicit ordering within an UpdateGroup
```C++
struct UpdateGroupA : ecs::IUpdateGroup {};

ECS_REGISTER_JOB_FOR_UPDATE_GROUP(ExampleJob, UpdateGroupA);

int main () {
    ecs::Manager mgr;
    mgr->RunUpdateGroup<UpdateGroupA>(0.0f /* dt */);
}
```

### Shared Components and Chunk Iteration
Useful when you can get large benefits from operating on entities in batches, such as rendering all entities with the same sprite/model.
Example renders 10,000 bullets in a 100x100 grid with a single draw call
```C++
namespace sprite {
struct Instance16x16 : ecs::ISharedComponent { Color Data[265]; }
}

struct ChunkRenderSprite16x16 : ecs::Job {
    ECS_READ(transform::Position, Pos);
    ECS_READ(sprite::Instance16x16, Sprite);

    void ForEachChunk (ecs::Timestep) override {
        transform::Position * posArray = Pos.GetChunkComponentArray();
        RenderSystem::RenderInstanced(Sprite->Data, posArray, GetChunkEntityCount());
    }
};

int main () {
    std::shared_ptr<sprite::Instance16x16> bulletModel = std::make_shared<sprite::Instance16x16>(LoadSprite(...));

    ecs::Manager mgr;
    for (int i = 0; i < 10000; ++i)
        mgr.CreateEntityImmediate(transform::Position{i % 100, i / 100}, bulletModel);

    mgr->RunJob<ChunkRenderSprite16x16>(0.0f);
}
```

### Queued Composition Changes from Jobs
Applied after completion of RunJob<> or RunUpdateGroup<>
```C++
struct QueuedChange : ecs::Job {
    ECS_READ(ecs::Entity, CurrentEntity);

    ECS_READ_SINGLETON(EnemyPrefab, Prefab);

    void ForEach (ecs::Timestep) override {
        QueueAddComponents(*CurrentEntity, ComponentA{1.0f});
        QueueRemoveComponents<ComponentB>(*CurrentEntity);

        QueueCreateEntity(ComponentA{1.0f});
        QueueDestroyEntity(*CurrentEntity);

        QueueCloneEntity(*CurrentEntity);
        QueueSpawnPrefab(Prefab->Value);
    }
};
```

### Prefabs
Useful for building a composition with default values that are commonly created
```C++
ecs::Prefab prefab = mgr.CreatePrefab(ComponentA{1.0f}, ComponentB{2.0f});
ecs::Entity spawned = mgr.SpawnPrefab(prefab);

mgr.HasComponent<ComponentA>(spawned);          // true
mgr.FindComponent<ComponentB>(spawned)->Value;  // 2.0f
```

### Configurable Settings
```C++
#define ECS_MAX_COMPONENTS 256
#define ECS_TIMESTEP_TYPE float
```

## TODO
Current Version: v0.9.2

Requirements for:
- v1.0.0
  - fix any additional known bugs
    - no known
  - add unit tests for any additional known edge cases
    - automate job tree validation
    - more complicated job tree unit tests

Potential future features
  - Batch operations (add/destroy/remove by filter/chunk)
  - Support for custom allocators

## License
See [LICENSE](LICENSE)
