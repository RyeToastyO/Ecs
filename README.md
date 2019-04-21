# Ecs
Entity Component System - A data-oriented solution for storing and processing state

## Current Usage
```C++
#include "ecs/ecs.h"

struct HealthCurrent {
    float Value;
};

struct HealthRegen {
    float Value;
};

struct RegenJob : public ecs::Job {
    ECS_WRITE(HealthCurrent, Current);
    ECS_READ(HealthRegen, Regen);

    void ForEach (ecs::Timestep dt) override {
        Current->Value += Regen->Value * dt;
    }
};

int main () {
    ecs::Manager mgr;
    ecs::Entity a = mgr.CreateEntityImmediate(HealthCurrent{10}, HealthRegen{1});

    mgr.RunJob<RegenJob>(1.0f /* dt */);

    std::cout << mgr.FindComponent<HealthCurrent>(a)->Value;
    /* 11 */
}
```

## Current Features
- Creation/destruction of entities
- Adding/setting/removing components
- Jobs that iterate all entities matching specified filter criteria:
  - Read/Write
  - Exclude/Require/RequireAny
  - Read/WriteOther (locks components but doesn't affect filtering)
  - Read/WriteSingleton (exactly one component of that type exists and isn't associated with an entity)
- Update groups
- Configurable:
  - Timestep type

## TODO
- Queued composition changes
- Job ordering control
- Multi-threading
- Add support for custom allocators

## License
See [LICENSE](LICENSE)
