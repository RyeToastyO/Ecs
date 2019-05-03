/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "helpers/token_combine.h"

namespace ecs {

#define ECS_RUN_THIS_AFTER(...) ::ecs::impl::RunAfter<__VA_ARGS__> ECS_TOKEN_COMBINE(__runAfter, __LINE__) = ::ecs::impl::RunAfter<__VA_ARGS__>(*this);
#define ECS_RUN_THIS_BEFORE(...) ::ecs::impl::RunBefore<__VA_ARGS__> ECS_TOKEN_COMBINE(__runBefore, __LINE__) = ::ecs::impl::RunBefore<__VA_ARGS__>(*this);

} // namespace ecs
