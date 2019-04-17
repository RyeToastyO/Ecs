# Ecs
Entity Component System - A data-oriented solution for storing and processing state

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
- Attempt to get rid of need for ECS_COMPONENT() macro
- Improve/replace ComponentFlags
- Add support for custom allocators

## License
See [LICENSE](LICENSE)
