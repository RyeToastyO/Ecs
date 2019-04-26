/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "../composition.h"

namespace ecs {

// Component Access
struct IComponentAccess {
    IComponentAccess (Job & job) : m_job(job) {}
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

template<typename T>
struct SingletonComponentAccess : public IComponentAccess {
    static_assert(std::is_base_of<ISingletonComponent, T>::value, "Can only access components that inherit ISingletonComponent");
    SingletonComponentAccess (Job & job) : IComponentAccess(job) {}
    void ApplyTo (ComponentFlags & flags) override { flags.SetFlags<T>(); }
    void UpdateManager () override { m_singletonComponent = this->m_job.m_manager->template GetSingletonComponent<T>(); }
protected:
    T * m_singletonComponent = nullptr;
};

// Actual component access
template<typename T, typename...Args>
struct Exclude : public CompositionAccess<T, Args...> {
    Exclude (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddExclude(this); }
};

template<typename T>
struct Read : public DataComponentAccess<T> {
    Read (Job & job) : DataComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddRead(this); }
    const T & operator* () const { return this->m_componentArray[this->m_job.m_currentIndex]; }
    const T * operator-> () const { return this->m_componentArray + this->m_job.m_currentIndex; }
};

template<typename T>
struct ReadOther : public LookupComponentAccess<T> {
    ReadOther (Job & job) : LookupComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddReadOther(this); }
    const T * operator[] (Entity entity) const { return this->m_job.m_manager->FindComponent<T>(entity); }
};

template<typename T>
struct ReadSingleton : public SingletonComponentAccess<T> {
    ReadSingleton (Job & job) : SingletonComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddReadSingleton(this); }
    const T & operator* () const { return this->m_singletonComponent; }
    const T * operator-> () const { return this->m_singletonComponent; }
};

template<typename T, typename...Args>
struct Require : public CompositionAccess<T, Args...> {
    Require (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddRequire(this); }
};

template<typename T, typename...Args>
struct RequireAny : public CompositionAccess<T, Args...> {
    RequireAny (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddRequireAny(this); }
};

template<typename T>
struct Write : public DataComponentAccess<T> {
    Write (Job & job) : DataComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddWrite(this); }
    T & operator* () const { return this->m_componentArray[this->m_job.m_currentIndex]; }
    T * operator-> () const { return this->m_componentArray + this->m_job.m_currentIndex; }
};

template<typename T>
struct WriteOther : public LookupComponentAccess<T> {
    WriteOther (Job & job) : LookupComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddWriteOther(this); }
    T * operator[] (Entity entity) const { return this->m_job.m_manager->FindComponent<T>(entity); }
};

template<typename T>
struct WriteSingleton : public SingletonComponentAccess<T> {
    WriteSingleton (Job & job) : SingletonComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddWriteSingleton(this); }
    T & operator* () const { return this->m_singletonComponent; }
    T * operator-> () const { return this->m_singletonComponent; }
};

}
