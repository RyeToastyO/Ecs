# Ecs
Entity Component System - A data-oriented solution for storing and processing state

## Current Usage
```C++
struct HealthCurrent {
    float Value;
};

struct HealthRegen {
    float Value;
};

struct RegenJob : public Job {
    ECS_WRITE(HealthCurrent, Current);
    ECS_READ(HealthRegen, Regen);

    void ForEach (float dt) override {
        Current->Value += Regen * dt;
    }
};
REGISTER_ECS_JOB(RegenJob);

int main () {
    Manager mgr;
    Entity a = mgr->CreateEntity(HealthCurrent{10}, HealthRegen{1});
    mgr->Update(1 /* dt */);

    cout << mgr->FindComponent<HealthCurrent>(a)->Value;
    /* 11 */
}
```

## Current Features
- Creation/destruction of entities
- Adding/setting/removing components
- Jobs that iterate all entities matching specified filter criteria:
  - Read/Write
  - Any/Exclude/Require
  - ReadOther/WriteOther (locks components but doesn't affect filtering)

## TODO
- Queued composition changes
- Singleton Components
- Job ordering control
- Multi-threading
- Improve/replace ComponentFlags
- Add support for custom allocators

## License
See [LICENSE](LICENSE)
