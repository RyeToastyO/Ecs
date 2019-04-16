#pragma once

#include <cstdint>
#include <vector>

#include "chunk.h"
#include "entity.h"

namespace ecs {

struct Job;
class Manager;

typedef uint32_t JobId;

// Interface
struct IComponentAccess {
    IComponentAccess (Job & job) : m_job(job) {}
    virtual void ApplyTo (ComponentFlags &) = 0;
    virtual void OnCreate () = 0;
    virtual void UpdateChunk (Chunk *) {}

protected:
    Job & m_job;
};

// Base types
template<typename T, typename...Args>
struct CompositionAccess : public IComponentAccess {
    CompositionAccess (Job & job) : IComponentAccess(job) {}
    void ApplyTo (ComponentFlags & flags) override { flags.SetFlags<T, Args...>(); }
};

template<typename T>
struct DataComponentAccess : public IComponentAccess {
    static_assert(!std::is_empty<T>(), "Cannot access an empty/tag component");
    DataComponentAccess (Job & job) : IComponentAccess(job) {}
    void ApplyTo (ComponentFlags & flags) override { flags.SetFlags<T>(); }
    void UpdateChunk (Chunk * chunk) { m_componentArray = chunk->Find<T>(); }
protected:
    T * m_componentArray = nullptr;
};

template<typename T>
struct LookupComponentAccess : public IComponentAccess {
    static_assert(!std::is_empty<T>(), "Cannot access an empty/tag component");
    LookupComponentAccess (Job & job) : IComponentAccess(job) {}
    void ApplyTo (ComponentFlags & flags) override { flags.SetFlags<T>(); }
};

// Actual component access
template<typename T, typename...Args>
struct Any : public CompositionAccess<T, Args...> {
    Any (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddAny(this); }
};
#define ECS_ANY(...) Any<__VA_ARGS__> __any##__LINE__ = Any<__VA_ARGS__>(*this);

template<typename T, typename...Args>
struct Exclude : public CompositionAccess<T, Args...> {
    Exclude (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddExclude(this); }
};
#define ECS_EXCLUDE(...) Exclude<__VA_ARGS__> __exclude##__LINE__ = Exclude<__VA_ARGS__>(*this);

template<typename T>
struct Read : public DataComponentAccess<T> {
    Read (Job & job) : DataComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddRead(this); }
    const T & operator* () const { return this->m_componentArray[this->m_job.GetCurrentIndex()]; }
    const T * operator-> () const { return this->m_componentArray + this->m_job.GetCurrentIndex(); }
};
#define ECS_READ(type, name)                                                                        \
Read<type> name = Read<type>(*this);                                                                \
static_assert(!std::is_empty<type>(), "Cannot read access an empty/tag component, use ECS_REQUIRE");

template<typename T>
struct ReadOther : public LookupComponentAccess<T> {
    ReadOther (Job & job) : LookupComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddReadOther(this); }
    const T * operator[] (Entity entity) const { return this->m_job.GetManager().FindComponent<T>(entity); }
};
#define ECS_READ_OTHER(type, name)                                                                              \
ReadOther<type> name = ReadOther<type>(*this);                                                                  \
static_assert(!std::is_empty<type>(), "Cannot read access an empty/tag component, use HasComponent<T>(entity)");

template<typename T, typename...Args>
struct Require : public CompositionAccess<T, Args...> {
    Require (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddRequire(this); }
};
#define ECS_REQUIRE(...) Require<__VA_ARGS__> __require##__LINE__ = Require<__VA_ARGS__>(*this);

template<typename T>
struct Write : public DataComponentAccess<T> {
    Write (Job & job) : DataComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddWrite(this); }
    T & operator* () const { return this->m_componentArray[this->m_job.GetCurrentIndex()]; }
    T * operator-> () const { return this->m_componentArray + this->m_job.GetCurrentIndex(); }
};
#define ECS_WRITE(type, name)                                                                           \
Write<type> name = Write<type>(*this);                                                                  \
static_assert(!std::is_empty<type>(), "Cannot write access an empty/tag component, use ECS_REQUIRE");

template<typename T>
struct WriteOther : public LookupComponentAccess<T> {
    WriteOther (Job & job) : LookupComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddWriteOther(this); }
    T * operator[] (Entity entity) const { return this->m_job.GetManager().FindComponent<T>(entity); }
};
#define ECS_WRITE_OTHER(type, name)                                                                                 \
WriteOther<type> name = WriteOther<type>(*this);                                                                    \
static_assert(!std::is_empty<type>(), "Cannot write access an empty/tag component, use HasComponent<T>(entity)");

// Job base class
struct Job {
    void OnChunkAdded (Chunk * chunk);
    void OnRegistered (Manager * manager);

    void AddAny (IComponentAccess * access);
    void AddExclude (IComponentAccess * access);
    void AddRead (IComponentAccess * access);
    void AddReadOther (IComponentAccess * access);
    void AddRequire (IComponentAccess * access);
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

    std::vector<ComponentFlags> m_any;
    ComponentFlags m_exclude;
    ComponentFlags m_required;

    ComponentFlags m_read;
    ComponentFlags m_write;

    Manager * m_manager = nullptr;
};

// This strategy of registration doesn't support multiple managers
const std::vector<Job *> & GetRegisteredJobs ();
JobId RegisterJob (Job * job);

#define REGISTER_ECS_JOB(type)              \
type s_##type;                              \
JobId s_##type##Id = RegisterJob(&s_##type);

}
