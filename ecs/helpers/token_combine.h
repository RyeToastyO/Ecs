#pragma once

namespace ecs {

#define ECS_TOKEN_COMBINE2(x, y) x##y
#define ECS_TOKEN_COMBINE(x, y) ECS_TOKEN_COMBINE2(x, y)

}
