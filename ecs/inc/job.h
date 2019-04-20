#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

#include "chunk.h"
#include "entity.h"

namespace ecs {

struct IComponentAccess;
struct Job;
class Manager;

// Job base class
struct Job {
    void OnChunkAdded (Chunk * chunk);
    void OnRegistered (Manager * manager);

    void AddExclude (IComponentAccess * access);
    void AddRead (IComponentAccess * access);
    void AddReadOther (IComponentAccess * access);
    void AddRequire (IComponentAccess * access);
    void AddRequireAny (IComponentAccess * access);
    void AddWrite (IComponentAccess * access);
    void AddWriteOther (IComponentAccess * access);

    uint32_t GetCurrentIndex () const { return m_currentIndex; }
    Manager & GetManager () const { return *m_manager; }

public:
    virtual void Run (float dt);
    virtual void ForEach (float dt) {}

private:
    bool IsValid (const Chunk * chunk) const;

private:
    uint32_t m_currentIndex = 0;
    std::vector<Chunk *> m_chunks;

    std::vector<IComponentAccess *> m_dataAccess;

    std::vector<ComponentFlags> m_requireAny;
    ComponentFlags m_exclude;
    ComponentFlags m_required;

    ComponentFlags m_read;
    ComponentFlags m_write;

    Manager * m_manager = nullptr;
};

// Register job for updates
// TODO: Split this into multiple types
//   - Update
//   - Manual run
//   - Prep/Commit phases?
#define REGISTER_ECS_JOB(type)              \
type s_##type;                              \
JobId s_##type##Id = RegisterJob(&s_##type);

// Component Access macros
#define ECS_EXCLUDE(...) Exclude<__VA_ARGS__> __exclude##__LINE__ = Exclude<__VA_ARGS__>(*this);

#define ECS_READ(componentType, variableName)                   \
Read<componentType> variableName = Read<componentType>(*this);  \
static_assert(!std::is_empty<componentType>(), "Cannot read access an empty/tag component, use ECS_REQUIRE");

#define ECS_READ_OTHER(componentType, variableName)                                                                         \
ReadOther<componentType> variableName = ReadOther<componentType>(*this);                                                    \
static_assert(!std::is_empty<componentType>(), "Cannot read access an empty/tag component, use HasComponent<T>(entity)");   \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "ReadOther Entity doesn't even make sense");

#define ECS_REQUIRE(...) Require<__VA_ARGS__> __require##__LINE__ = Require<__VA_ARGS__>(*this);
#define ECS_REQUIRE_ANY(...) RequireAny<__VA_ARGS__> __requireAny##__LINE__ = RequireAny<__VA_ARGS__>(*this);

#define ECS_WRITE(componentType, variableName)                                                                  \
Write<componentType> variableName = Write<componentType>(*this);                                                \
static_assert(!std::is_empty<componentType>(), "Cannot write access an empty/tag component, use ECS_REQUIRE");  \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "Don't write to Entity, you will break everything");

#define ECS_WRITE_OTHER(componentType, variableName)                                                                        \
WriteOther<componentType> variableName = WriteOther<componentType>(*this);                                                  \
static_assert(!std::is_empty<componentType>(), "Cannot write access an empty/tag component, use HasComponent<T>(entity)");  \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "Don't write to Entity, you will break everything");

}

#include "inline/job.inl"
