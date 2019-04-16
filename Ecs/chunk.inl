namespace ecs {

template<typename T>
T * Chunk::Find () {
    // Give a valid pointer if a tag component is requested, but don't
    // bother looking it up in the component arrays since we didn't allocate memory for it
    if (std::is_empty<T>())
        return m_composition.Has<T>() ? reinterpret_cast<T*>(m_componentMemory) : nullptr;

    auto iter = m_componentArrays.find(T::GetId());
    if (iter == m_componentArrays.end())
        return nullptr;
    return iter->second.As<T>();
}

template<typename T>
T * Chunk::Find (uint32_t index) {
    if (index >= m_count)
        return nullptr;

    T * arrayStart = Find<T>();
    return arrayStart ? arrayStart + index : nullptr;
}

}
