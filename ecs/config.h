/*
 * Copyright (c) 2020 Riley Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

namespace ecs {

// This can be large during development.
// Once you actually know your shipping component count,
// this should be set to it to speed up ComponentFlags
#ifndef ECS_MAX_COMPONENTS
#define ECS_MAX_COMPONENTS 256
#endif

}
