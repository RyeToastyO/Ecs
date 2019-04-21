#include "../component_access.h"

namespace ecs {

// Job
void Job::AddExclude (IComponentAccess * access) {
    access->ApplyTo(m_exclude);
}

void Job::AddRead (IComponentAccess * access) {
    access->ApplyTo(m_read);
    access->ApplyTo(m_required);
    m_dataAccess.push_back(access);
}

void Job::AddReadOther (IComponentAccess * access) {
    access->ApplyTo(m_read);
}

void Job::AddReadSingleton (IComponentAccess * access) {
    access->ApplyTo(m_read);
    m_singletonAccess.push_back(access);
}

void Job::AddRequire (IComponentAccess * access) {
    access->ApplyTo(m_required);
}

void Job::AddRequireAny (IComponentAccess * access) {
    ComponentFlags any;
    access->ApplyTo(any);
    m_requireAny.push_back(any);
}

void Job::AddWrite (IComponentAccess * access) {
    access->ApplyTo(m_write);
    access->ApplyTo(m_required);
    m_dataAccess.push_back(access);
}

void Job::AddWriteOther (IComponentAccess * access) {
    access->ApplyTo(m_write);
}

void Job::AddWriteSingleton (IComponentAccess * access) {
    access->ApplyTo(m_write);
    m_singletonAccess.push_back(access);
}

template<typename T>
bool Job::HasComponent (Entity entity) const {
    return m_manager->HasComponent<T>(entity);
}

void Job::OnChunkAdded (Chunk * chunk) {
    if (!IsValid(chunk))
        return;
    m_chunks.push_back(chunk);
}

void Job::OnRegistered (Manager * manager) {
    m_manager = manager;
    m_chunks.clear();

    for (auto singletonAccess : m_singletonAccess)
        singletonAccess->UpdateManager();
}

bool Job::IsValid (const Chunk * chunk) const {
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

void Job::Run (Timestep dt) {
    for (auto chunk : m_chunks) {
        for (auto dataAccess : m_dataAccess)
            dataAccess->UpdateChunk(chunk);
        for (m_currentIndex = 0; m_currentIndex < chunk->GetCount(); ++m_currentIndex)
            ForEach(dt);
    }
}

// Registration
struct JobRegistry {
    static JobId RegisterJob ();
};

JobId JobRegistry::RegisterJob () {
    static JobId s_idCounter = 0;
    return s_idCounter++;
}

template<typename T>
struct JobIdGetter {
    static JobId GetId ();
};

template<typename T>
JobId JobIdGetter<T>::GetId () {
    static JobId id = JobRegistry::RegisterJob();
    return id;
}

template<typename T>
static JobId GetJobId () {
    static_assert(std::is_base_of<Job, T>::value, "Must inherit Job to use GetJobId");
    return JobIdGetter<typename std::remove_const<T>::type>::GetId();
}

} // namespace ecs
