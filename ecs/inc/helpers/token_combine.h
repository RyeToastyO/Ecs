/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

namespace ecs {

#define ECS_TOKEN_COMBINE2(x, y) x##y
#define ECS_TOKEN_COMBINE(x, y) ECS_TOKEN_COMBINE2(x, y)

}
