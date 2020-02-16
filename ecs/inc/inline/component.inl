/*
 * Copyright (c) 2020 Riley Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include <cassert>

namespace ecs {
namespace impl {

inline std::unordered_map<ComponentId, size_t> & ComponentRegistry::GetComponentSizeMap () {
    static std::unordered_map<ComponentId, size_t> s_sizeMap;
    return s_sizeMap;
}
inline ComponentId & ComponentRegistry::GetComponentCounter () {
    static ComponentId s_componentCount = 0;
    return s_componentCount;
}
inline size_t ComponentRegistry::GetComponentSize (ComponentId id) {
    auto & sizeMap = GetComponentSizeMap();
    const auto iter = sizeMap.find(id);
    assert(iter != sizeMap.end());
    return iter->second;
}

inline ComponentId ComponentRegistry::RegisterComponent (size_t size) {
    static ComponentId s_idCounter = 0;
    GetComponentSizeMap().emplace(s_idCounter, size);
    return s_idCounter++;
}

template<typename T>
struct Component {
    static ComponentId GetId ();
};

template<typename T>
inline ComponentId Component<T>::GetId () {
    // We say a shared component is a size of 0, since 0 memory is allocated for them per entity
    static ComponentId id = ComponentRegistry::RegisterComponent(std::is_base_of<ISharedComponent, T>::value || std::is_empty<T>() ? 0 : sizeof(T));
    assert(id < ECS_MAX_COMPONENTS);
    return id;
}

template<typename T>
inline static ComponentId GetComponentId () {
    return Component<typename std::remove_const<T>::type>::GetId();
}

inline static size_t GetComponentSize (ComponentId id) {
    return ComponentRegistry::GetComponentSize(id);
}

} // namespace impl
} // namespace ecs
