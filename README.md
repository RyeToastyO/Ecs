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

### Chunk Iteration
Useful when you can operate on entities in batches, such as rendering all entities with the same sprite/model.
Will be more useful once shared components are implemented so you can store a pointer to the bullet model as a component.
```C++
struct ChunkRender : ecs::Job {
    ECS_READ(transform::Position, Pos);
    ECS_REQUIRE(model::IsBullet);

    void ForEachChunk (ecs::Timestep) override {
        transform::Position * posArray = Pos.GetChunkComponentArray();
        RenderSystem::RenderBullets(posArray, GetChunkEntityCount());
    }
};
```

### Queued Composition Changes from Jobs
Applied after completion of RunJob<> or RunUpdateGroup<>
```C++
struct QueuedChange : ecs::Job {
    ECS_READ(ecs::Entity, CurrentEntity);

    void ForEach (ecs::Timestep) override {
        QueueAddComponents(*CurrentEntity, ComponentA{1.0f});
        QueueRemoveComponents<ComponentB>(*CurrentEntity);
        QueueCreateEntity(ComponentA{1.0f});
        QueueDestroyEntity(*CurrentEntity);
    }
};
```

### Configurable Settings
```C++
#define ECS_MAX_COMPONENTS 256
#define ECS_TIMESTEP_TYPE float
```

## TODO
Current Version: v0.8.3

Requirements for:
- v0.9.0
  - shared components
- v1.0.0
  - fix any additional known bugs
    - no known
  - add unit tests for any additional known edge cases
    - changing composition of an entity in the middle of chunk
    - automate job tree validation
    - more complicated job tree unit tests

Potential future features
  - Support for custom allocators

## License
See [LICENSE](LICENSE)
