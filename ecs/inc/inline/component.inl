// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#include <cassert>

namespace ecs {
namespace impl {

inline std::vector<size_t>& ComponentRegistry::GetComponentSizes () {
    static std::vector<size_t> s_sizes;
    return s_sizes;
}
inline size_t ComponentRegistry::GetComponentSize (ComponentId id) {
    auto& sizes = GetComponentSizes();
    assert(id < sizes.size());
    return sizes[id];
}

inline ComponentId ComponentRegistry::RegisterComponent (size_t size) {
    static ComponentId s_idCounter = 0;
    GetComponentSizes().push_back(size);
    assert(s_idCounter < ECS_MAX_COMPONENTS);
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
