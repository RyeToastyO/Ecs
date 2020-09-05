// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

namespace ecs {
namespace impl {

template<typename T>
inline ComponentId GetComponentId () {
    // If you get a compile error on this function,
    // it means you forgot ECS_COMPONENT(name) macro in your component struct
    return T::GetEcsComponentId();
}

template<typename T>
inline size_t GetComponentSize () {
    return std::is_empty<T>() ? 0 : sizeof(T);
}

} // namespace impl
} // namespace ecs
