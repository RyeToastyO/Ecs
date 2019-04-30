/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "../component_access.h"

namespace ecs {

// Job
inline void Job::AddExclude (IComponentAccess * access) {
    access->ApplyTo(m_exclude);
}

inline void Job::AddRead (IComponentAccess * access) {
    access->ApplyTo(m_read);
    access->ApplyTo(m_required);
    m_dataAccess.push_back(access);
}

inline void Job::AddReadOther (IComponentAccess * access) {
    access->ApplyTo(m_read);
}

inline void Job::AddReadSingleton (IComponentAccess * access) {
    access->ApplyTo(m_read);
    m_singletonAccess.push_back(access);
}

inline void Job::AddRequire (IComponentAccess * access) {
    access->ApplyTo(m_required);
}

inline void Job::AddRequireAny (IComponentAccess * access) {
    ComponentFlags any;
    access->ApplyTo(any);
    m_requireAny.push_back(any);
}

inline void Job::AddWrite (IComponentAccess * access) {
    access->ApplyTo(m_write);
    access->ApplyTo(m_required);
    m_dataAccess.push_back(access);
}

inline void Job::AddWriteOther (IComponentAccess * access) {
    access->ApplyTo(m_write);
}

inline void Job::AddWriteSingleton (IComponentAccess * access) {
    access->ApplyTo(m_write);
    m_singletonAccess.push_back(access);
}

inline void Job::ApplyQueuedCommands () {
    m_commands.Apply(m_manager);
}

inline const ComponentFlags & Job::GetReadFlags () const {
    return m_read;
}

inline const ComponentFlags & Job::GetWriteFlags () const {
    return m_write;
}

template<typename T>
inline bool Job::HasComponent (Entity entity) const {
    return m_manager->HasComponent<T>(entity);
}

inline void Job::OnChunkAdded (Chunk * chunk) {
    if (!IsValid(chunk))
        return;
    m_chunks.push_back(chunk);
}

inline void Job::OnRegistered (Manager * manager) {
    m_manager = manager;
    m_chunks.clear();

    for (auto singletonAccess : m_singletonAccess)
        singletonAccess->UpdateManager();
}

inline bool Job::IsValid (const Chunk * chunk) const {
    const auto & composition = chunk->GetComposition();
    if (!composition.HasAll(m_required))
        return false;
    if (!composition.HasNone(m_exclude))
        return false;
    for (const auto & any : m_requireAny) {
        if (!composition.HasAny(any))
            return false;
    }
    return true;
}

inline void Job::Run (Timestep dt) {
    for (auto chunk : m_chunks) {
        for (auto dataAccess : m_dataAccess)
            dataAccess->UpdateChunk(chunk);
        for (m_currentIndex = 0; m_currentIndex < chunk->GetCount(); ++m_currentIndex)
            ForEach(dt);
    }
}

template<typename T, typename...Args>
inline void Job::QueueAddComponents (Entity entity, T component, Args...args) {
    m_commands.AddComponents(entity, component, args...);
}

template<typename T, typename...Args>
inline void Job::QueueCreateEntity (T component, Args...args) {
    m_commands.CreateEntity(component, args...);
}

inline void Job::QueueDestroyEntity (Entity entity) {
    m_commands.DestroyEntity(entity);
}

template<typename T, typename...Args>
inline void Job::QueueRemoveComponents (Entity entity) {
    m_commands.RemoveComponents<T, Args...>(entity);
}

// Registration
struct JobRegistry {
    static JobId RegisterJob ();
};

inline JobId JobRegistry::RegisterJob () {
    static JobId s_idCounter = 0;
    return s_idCounter++;
}

template<typename T>
struct JobIdGetter {
    static JobId GetId ();
};

template<typename T>
inline JobId JobIdGetter<T>::GetId () {
    static JobId id = JobRegistry::RegisterJob();
    return id;
}

template<typename T>
inline static JobId GetJobId () {
    static_assert(std::is_base_of<Job, T>::value, "Must inherit Job to use GetJobId");
    return JobIdGetter<typename std::remove_const<T>::type>::GetId();
}

} // namespace ecs
