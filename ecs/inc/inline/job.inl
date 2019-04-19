namespace ecs {

// Component Access
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

template<typename T, typename...Args>
struct Exclude : public CompositionAccess<T, Args...> {
    Exclude (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddExclude(this); }
};

template<typename T>
struct Read : public DataComponentAccess<T> {
    Read (Job & job) : DataComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddRead(this); }
    const T & operator* () const { return this->m_componentArray[this->m_job.GetCurrentIndex()]; }
    const T * operator-> () const { return this->m_componentArray + this->m_job.GetCurrentIndex(); }
};

template<typename T>
struct ReadOther : public LookupComponentAccess<T> {
    ReadOther (Job & job) : LookupComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddReadOther(this); }
    const T * operator[] (Entity entity) const { return this->m_job.GetManager().FindComponent<T>(entity); }
};

template<typename T, typename...Args>
struct Require : public CompositionAccess<T, Args...> {
    Require (Job & job) : CompositionAccess<T, Args...>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddRequire(this); }
};

template<typename T>
struct Write : public DataComponentAccess<T> {
    Write (Job & job) : DataComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddWrite(this); }
    T & operator* () const { return this->m_componentArray[this->m_job.GetCurrentIndex()]; }
    T * operator-> () const { return this->m_componentArray + this->m_job.GetCurrentIndex(); }
};

template<typename T>
struct WriteOther : public LookupComponentAccess<T> {
    WriteOther (Job & job) : LookupComponentAccess<T>(job) { OnCreate(); }
    void OnCreate () override { this->m_job.AddWriteOther(this); }
    T * operator[] (Entity entity) const { return this->m_job.GetManager().FindComponent<T>(entity); }
};

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
static std::vector<Job *> & GetRegisteredJobs () {
    static std::vector<Job *> s_registeredJobs;
    return s_registeredJobs;
}

typedef uint32_t JobId;
static JobId RegisterJob (Job * job) {
    static JobId s_jobId = 0;
    GetRegisteredJobs().push_back(job);
    return s_jobId++;
}

} // namespace ecs
