#pragma once

#include <cassert>
#include <cstdint>
#include <unordered_map>

namespace ecs {

// This can be large during development.
// Once you actually know your shipping component count,
// this should be set to it to speed up ComponentFlags
#ifndef MAX_ECS_COMPONENTS
#define MAX_ECS_COMPONENTS 256
#endif

typedef uint32_t ComponentId;

struct ComponentRegistry {
    static std::unordered_map<ComponentId, size_t> & GetComponentSizeMap () {
        static std::unordered_map<ComponentId, size_t> s_sizeMap;
        return s_sizeMap;
    }
    static ComponentId & GetComponentCounter () {
        static ComponentId s_componentCount = 0;
        return s_componentCount;
    }
    static size_t GetComponentSize (ComponentId id) {
        auto & sizeMap = GetComponentSizeMap();
        const auto iter = sizeMap.find(id);
        assert(iter != sizeMap.end());
        return iter->second;
    }

    static ComponentId RegisterComponent (size_t size) {
        static ComponentId s_idCounter = 0;
        GetComponentSizeMap().emplace(s_idCounter, size);
        return s_idCounter++;
    }
};

template<typename T>
struct Component {
    static ComponentId GetId ();
};

template<typename T>
ComponentId Component<T>::GetId () {
    static ComponentId id = ComponentRegistry::RegisterComponent(std::is_empty<T>() ? 0 : sizeof(T));
    assert(id < MAX_ECS_COMPONENTS);
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
