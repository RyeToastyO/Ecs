namespace ecs {

bool Entity::operator== (const Entity & rhs) const {
    return index == rhs.index && generation == rhs.generation;
}

} // namespace ecs
