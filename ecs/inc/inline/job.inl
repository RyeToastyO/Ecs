/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

namespace ecs {

// Job
inline void Job::AddExclude (impl::IComponentAccess * access) {
    access->ApplyTo(m_exclude);
}

inline void Job::AddRead (impl::IComponentAccess * access) {
    access->ApplyTo(m_read);
    access->ApplyTo(m_required);
    m_dataAccess.push_back(access);
}

inline void Job::AddReadOther (impl::IComponentAccess * access) {
    access->ApplyTo(m_read);
}

inline void Job::AddReadSingleton (impl::IComponentAccess * access) {
    access->ApplyTo(m_read);
    m_singletonAccess.push_back(access);
}

inline void Job::AddRequire (impl::IComponentAccess * access) {
    access->ApplyTo(m_required);
}

inline void Job::AddRequireAny (impl::IComponentAccess * access) {
    impl::ComponentFlags any;
    access->ApplyTo(any);
    m_requireAny.push_back(any);
}

inline void Job::AddWrite (impl::IComponentAccess * access) {
    access->ApplyTo(m_write);
    access->ApplyTo(m_required);
    m_dataAccess.push_back(access);
}

inline void Job::AddWriteOther (impl::IComponentAccess * access) {
    access->ApplyTo(m_write);
}

inline void Job::AddWriteSingleton (impl::IComponentAccess * access) {
    access->ApplyTo(m_write);
    m_singletonAccess.push_back(access);
}

inline void Job::AddRunAfter (impl::IJobOrdering * ordering) {
    ordering->ApplyTo(m_runAfter);
}

inline void Job::AddRunBefore (impl::IJobOrdering * ordering) {
    ordering->ApplyTo(m_runBefore);
}

inline void Job::ApplyQueuedCommands () {
    m_commands.Apply(m_manager);
}

inline const std::unordered_set<impl::JobId> & Job::GetRunAfter () const {
    return m_runAfter;
}

inline const std::unordered_set<impl::JobId> & Job::GetRunBefore () const {
    return m_runBefore;
}

inline const impl::ComponentFlags & Job::GetReadFlags () const {
    return m_read;
}

inline const impl::ComponentFlags & Job::GetWriteFlags () const {
    return m_write;
}

template<typename T>
inline bool Job::HasComponent (Entity entity) const {
    return m_manager->HasComponent<T>(entity);
}

inline void Job::OnChunkAdded (impl::Chunk * chunk) {
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

inline bool Job::IsValid (const impl::Chunk * chunk) const {
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
namespace impl {

struct JobRegistry {
    static impl::JobId RegisterJob ();
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
inline JobId GetJobId () {
    static_assert(std::is_base_of<Job, T>::value, "Must inherit Job to use GetJobId");
    return JobIdGetter<typename std::remove_const<T>::type>::GetId();
}

} // namespace impl

} // namespace ecs
