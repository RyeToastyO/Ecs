#include "pch.h"

#include "job.h"

namespace ecs {

// Job
void Job::AddAny (IComponentAccess * access) {
    ComponentFlags any;
    access->ApplyTo(any);
    m_any.push_back(any);
}

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

void Job::AddRequire (IComponentAccess * access) {
    access->ApplyTo(m_required);
}

void Job::AddWrite (IComponentAccess * access) {
    access->ApplyTo(m_write);
    access->ApplyTo(m_required);
    m_dataAccess.push_back(access);
}

void Job::AddWriteOther (IComponentAccess * access) {
    access->ApplyTo(m_write);
}

void Job::OnChunkAdded (Chunk * chunk) {
    if (!IsValid(chunk))
        return;
    m_chunks.push_back(chunk);
}

void Job::OnRegistered (Manager * manager) {
    m_manager = manager;
    m_chunks.clear();
}

bool Job::IsValid (const Chunk * chunk) const {
    const auto & composition = chunk->GetComposition();
    if (!composition.HasAll(m_required))
        return false;
    if (!composition.HasNone(m_exclude))
        return false;
    for (const auto & any : m_any) {
        if (!composition.HasAny(any))
            return false;
    }
    return true;
}

void Job::Run (float dt) {
    for (auto chunk : m_chunks) {
        for (auto dataAccess : m_dataAccess)
            dataAccess->UpdateChunk(chunk);
        for (m_currentIndex = 0; m_currentIndex < chunk->GetCount(); ++m_currentIndex)
            ForEach(dt);
    }
}

// Registration
JobId s_jobId = 0;
ARRAY(Job *) s_registeredJobs;
const ARRAY(Job *) & GetRegisteredJobs () {
    return s_registeredJobs;
}

JobId RegisterJob (Job * job) {
    s_registeredJobs.push_back(job);
    return s_jobId++;
}

} // namespace ecs
