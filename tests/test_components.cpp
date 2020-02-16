/*
 * Copyright (c) 2020 Riley Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "test_components.h"

namespace test {

std::shared_ptr<SharedA> SharedA1 = std::make_shared<SharedA>(1);
std::shared_ptr<SharedA> SharedA2 = std::make_shared<SharedA>(2);
std::shared_ptr<SharedB> SharedB1 = std::make_shared<SharedB>(1);
std::shared_ptr<SharedB> SharedB2 = std::make_shared<SharedB>(2);

} // namespace test
