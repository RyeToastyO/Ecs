/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "../composition.h"

namespace ecs {
namespace impl {

// Component Access
struct IComponentAccess {
    inline IComponentAccess (Job & job) : m_job(job) {}
    virtual void ApplyTo (ComponentFlags &) = 0;
    virtual void OnCreate () = 0;
    virtual void UpdateChunk (Chunk *) {}
    virtual void UpdateManager () {}

protected:
    Job & m_job;
};

// Base types
template<typename T, typename...Args>
struct CompositionAccess : public IComponentAccess {
    inline CompositionAccess (Job & job) : IComponentAccess(job) {}
    inline void ApplyTo (ComponentFlags & flags) override { flags.SetFlags<T, Args...>(); }
};

template<typename T>
struct DataComponentAccess : public IComponentAccess {
    static_assert(!std::is_empty<T>(), "Cannot access an empty/tag component");
    inline DataComponentAccess (Job & job) : IComponentAccess(job) {}
    inline void ApplyTo (ComponentFlags & flags) override { flags.SetFlags<T>(); }
    inline void UpdateChunk (Chunk * chunk) { m_componentArray = chunk->Find<T>(); }
protected:
    T * m_componentArray = nullptr;
};

template<typename T>
struct LookupComponentAccess : public IComponentAccess {
    static_assert(!std::is_empty<T>(), "Cannot access an empty/tag component");
    inline LookupComponentAccess (Job & job) : IComponentAccess(job) {}
    inline void ApplyTo (ComponentFlags & flags) override { flags.SetFlags<T>(); }
};

template<typename T>
struct SingletonComponentAccess : public IComponentAccess {
    static_assert(std::is_base_of<ISingletonComponent, T>::value, "Can only access components that inherit ISingletonComponent");
    inline SingletonComponentAccess (Job & job) : IComponentAccess(job) {}
    inline void ApplyTo (ComponentFlags & flags) override { flags.SetFlags<T>(); }
    inline void UpdateManager () override { m_singletonComponent = this->m_job.m_manager->template GetSingletonComponent<T>(); }
protected:
    T * m_singletonComponent = nullptr;
};

// Actual component access
template<typename T, typename...Args>
struct Exclude : public CompositionAccess<T, Args...> {
    inline Exclude (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    inline void OnCreate () override { this->m_job.AddExclude(this); }
};

template<typename T>
struct Read : public DataComponentAccess<T> {
    inline Read (Job & job) : DataComponentAccess<T>(job) { OnCreate(); }
    inline void OnCreate () override { this->m_job.AddRead(this); }
    inline const T & operator* () const { return this->m_componentArray[this->m_job.m_currentIndex]; }
    inline const T * operator-> () const { return this->m_componentArray + this->m_job.m_currentIndex; }
};

template<typename T>
struct ReadOther : public LookupComponentAccess<T> {
    inline ReadOther (Job & job) : LookupComponentAccess<T>(job) { OnCreate(); }
    inline void OnCreate () override { this->m_job.AddReadOther(this); }
    inline const T * operator[] (Entity entity) const { return this->m_job.m_manager->FindComponent<T>(entity); }
};

template<typename T>
struct ReadSingleton : public SingletonComponentAccess<T> {
    inline ReadSingleton (Job & job) : SingletonComponentAccess<T>(job) { OnCreate(); }
    inline void OnCreate () override { this->m_job.AddReadSingleton(this); }
    inline const T & operator* () const { return this->m_singletonComponent; }
    inline const T * operator-> () const { return this->m_singletonComponent; }
};

template<typename T, typename...Args>
struct Require : public CompositionAccess<T, Args...> {
    inline Require (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    inline void OnCreate () override { this->m_job.AddRequire(this); }
};

template<typename T, typename...Args>
struct RequireAny : public CompositionAccess<T, Args...> {
    inline RequireAny (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    inline void OnCreate () override { this->m_job.AddRequireAny(this); }
};

template<typename T>
struct Write : public DataComponentAccess<T> {
    inline Write (Job & job) : DataComponentAccess<T>(job) { OnCreate(); }
    inline void OnCreate () override { this->m_job.AddWrite(this); }
    inline T & operator* () const { return this->m_componentArray[this->m_job.m_currentIndex]; }
    inline T * operator-> () const { return this->m_componentArray + this->m_job.m_currentIndex; }
};

template<typename T>
struct WriteOther : public LookupComponentAccess<T> {
    inline WriteOther (Job & job) : LookupComponentAccess<T>(job) { OnCreate(); }
    inline void OnCreate () override { this->m_job.AddWriteOther(this); }
    inline T * operator[] (Entity entity) const { return this->m_job.m_manager->FindComponent<T>(entity); }
};

template<typename T>
struct WriteSingleton : public SingletonComponentAccess<T> {
    inline WriteSingleton (Job & job) : SingletonComponentAccess<T>(job) { OnCreate(); }
    inline void OnCreate () override { this->m_job.AddWriteSingleton(this); }
    inline T & operator* () const { return this->m_singletonComponent; }
    inline T * operator-> () const { return this->m_singletonComponent; }
};

} // namespace impl
} // namespace ecs
