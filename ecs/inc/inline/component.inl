/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include <cassert>

namespace ecs {
namespace impl {

inline std::unordered_map<ComponentId, ComponentDestructor> & ComponentRegistry::GetComponentDestructorMap () {
    static std::unordered_map<ComponentId, ComponentDestructor> s_destructorMap;
    return s_destructorMap;
}
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

inline ComponentId ComponentRegistry::RegisterComponent (size_t size, ComponentDestructor destructor) {
    static ComponentId s_idCounter = 0;
    GetComponentSizeMap().emplace(s_idCounter, size);
    GetComponentDestructorMap().emplace(s_idCounter, destructor);
    return s_idCounter++;
}

template<typename T>
struct Component {
    static ComponentId GetId ();
};

template<typename T>
inline ComponentId Component<T>::GetId () {
    static ComponentId id = ComponentRegistry::RegisterComponent(
        std::is_empty<T>() ? 0 : sizeof(T),
        [](void * component) { reinterpret_cast<T*>(component)->~T(); }
    );
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

inline static void DestructComponent (ComponentId id, void * component) {
    ComponentRegistry::GetComponentDestructorMap().find(id)->second(component);
}

} // namespace impl
} // namespace ecs
