#include "../public/component.h"

#include <unordered_map>

namespace ecs {

ComponentId s_componentId = 0;

ComponentId AllocateComponentId () {
    return s_componentId++;
}

std::unordered_map<ComponentId, size_t> s_componentSizes;

size_t GetComponentSize (ComponentId id) {
    return s_componentSizes.find(id)->second;
}

size_t RegisterComponentSize (ComponentId id, size_t size) {
    s_componentSizes.emplace(id, size);
    return size;
}

} // namespace ecs
