#include <cassert>

namespace ecs {

std::unordered_map<ComponentId, size_t> & ComponentRegistry::GetComponentSizeMap () {
    static std::unordered_map<ComponentId, size_t> s_sizeMap;
    return s_sizeMap;
}
ComponentId & ComponentRegistry::GetComponentCounter () {
    static ComponentId s_componentCount = 0;
    return s_componentCount;
}
size_t ComponentRegistry::GetComponentSize (ComponentId id) {
    auto & sizeMap = GetComponentSizeMap();
    const auto iter = sizeMap.find(id);
    assert(iter != sizeMap.end());
    return iter->second;
}

ComponentId ComponentRegistry::RegisterComponent (size_t size) {
    static ComponentId s_idCounter = 0;
    GetComponentSizeMap().emplace(s_idCounter, size);
    return s_idCounter++;
}

template<typename T>
struct Component {
    static ComponentId GetId ();
};

template<typename T>
ComponentId Component<T>::GetId () {
    static ComponentId id = ComponentRegistry::RegisterComponent(std::is_empty<T>() ? 0 : sizeof(T));
    assert(id < ECS_MAX_COMPONENTS);
    return id;
}

template<typename T>
static ComponentId GetComponentId () {
    return Component<typename std::remove_const<T>::type>::GetId();
}

static size_t GetComponentSize (ComponentId id) {
    return ComponentRegistry::GetComponentSize(id);
}

} // namespace ecs
