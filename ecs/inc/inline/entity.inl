// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

namespace ecs {

inline bool Entity::operator== (const Entity& rhs) const {
    return index == rhs.index && generation == rhs.generation;
}

inline bool Entity::operator!= (const Entity& rhs) const {
    return !(*this == rhs);
}

inline EntityId Entity::GetId () const {
    return (static_cast<EntityId>(index) << 32) | generation;
}

inline Entity Entity::FromId (EntityId id) {
    Entity ret;
    ret.index = static_cast<uint32_t>((id >> 32) & UINT32_MAX);
    ret.generation = static_cast<uint32_t>(id & UINT32_MAX);

    return ret;
}

} // namespace ecs

namespace std {
    template <> struct hash<::ecs::Entity> {
        size_t operator() (const ::ecs::Entity& entity) const {
            std::hash<ecs::EntityId> hasher;
            return hasher(entity.GetId());
        }
    };
};
