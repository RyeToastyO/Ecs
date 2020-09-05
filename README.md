# Ecs
Entity Component System - A data-oriented solution for storing and processing state

### Usage Example
```C++
#include "ecs/ecs.h"

namespace health {
struct Current {
    ECS_COMPONENT(CurrentHealth);
    float Value;
};
struct Regen {
    ECS_COMPONENT(HealthRegen);
    float Value;
};
}

struct RegenJob : ecs::Job {
    ECS_WRITE(health::Current, Current);
    ECS_READ(health::Regen, Regen);

    void ForEach () override {
        Current->Value += Regen->Value;
    }
};

int main () {
    ecs::Manager mgr;
    ecs::Entity a = mgr.CreateEntityImmediate(health::Current{10}, health::Regen{1});

    mgr.RunJob<RegenJob>();

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
Note: Standard components must be plain old data. They do not run destructors and are moved using direct memcpy
```C++
mgr.AddComponents(entity, ComponentC{30.0f}, ComponentD{1});
mgr.RemoveComponents<ComponentA, ComponentB>(entity);
mgr.FindComponent<ComponentC>(entity)->Value = 5.0f;
mgr.HasComponent<ComponentD>(entity) == true;
```

### Singleton Components
Note: Singleton Components do run destructors
```C++
struct CameraPosition : ecs::ISingletonComponent {
    ECS_COMPONENT(CameraPosition);
    float3 Value;
};

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

    int m_internalData = 0;
    float m_timestep = 1.0f / 60;

    void Run () override {
        m_internalData = K->Value;

        // Executes the ForEach on each entity that matches all
        // WRITE/READ/REQUIRE/EXCLUDE/REQUIRE_ANY filters
        Job::Run();

        L->Value = m_internalData;
    }

    void ForEach () override {
        if (ComponentI* i = I.Find(B->Value))
            A->Value += i->Value * m_timestep;
        if (ComponentJ* j = J.Find(B->Value))
            j->Value = A->Value;
        m_internalData += A->Value;
    }
};

int main () {
    Manager mgr;
    mgr->RunJob<ExampleJob>();
}
```

### Chunk Iteration
Useful when you can get large benefits from operating on entities in batches, such as rendering all entities with the same sprite/model.
Example renders 10,000 bullets in a 100x100 grid with a single draw call
```C++
namespace sprite {
struct Textures : ecs::ISingletonComponent {
    ECS_COMPONENT(SpriteTextures);
    Sprite Bullet;
}

struct TagBullet { ECS_COMPONENT(BulletSprite); }
}

struct ChunkRenderBullets : ecs::Job {
    ECS_READ_SINGLETON(sprite::Textures, Textures);
    ECS_READ(transform::Position, Pos);
    ECS_REQUIRE(sprite::TagBullet);

    void ForEachChunk () override {
        transform::Position* posArray = Pos.GetChunkComponentArray();
        RenderSystem::RenderInstanced(Textures->Bullet, posArray, GetChunkEntityCount());
    }
};

int main () {
    ecs::Manager mgr;

    mgr.GetSingletonComponent<sprite::Textures>()->Bullet = LoadTexture("bullet.bmp");

    for (int i = 0; i < 10000; ++i)
        mgr.CreateEntityImmediate(transform::Position{i % 100, i / 100}, sprite::TagBullet{});

    mgr->RunJob<ChunkRenderBullets>();
}
```

### Queued Composition Changes from Jobs
Applied after completion of RunJob<>
```C++
struct QueuedChange : ecs::Job {
    ECS_READ(ecs::Entity, CurrentEntity);

    ECS_READ_SINGLETON(EnemyPrefab, Prefab);

    void ForEach () override {
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
// Currently none
```

## TODO
Current Version: v2.0.3

Potential future features
  - Running destructors and assignment operators for components
  - Batch operations (add/destroy/remove by filter/chunk)
  - QueueCreate/Spawn should return an entity to act on
  - Improved cache alignment of chunks
  - Breaking chunk alocations into cache line sizes
  - CanMultithread<JobA, JobB>() helper
  - RunJobsMultithreaded<JobA, JobB>() helper
  - PruneUnusedChunks(time since empty)
  - Support for custom allocators

## License
See [LICENSE](LICENSE)
