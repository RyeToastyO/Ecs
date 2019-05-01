/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "test_correctness.h"
#include "test_multi_threading.h"
#include "test_speed.h"

namespace test {

// Static data
int s_errorCount = 0;
std::mutex s_errorLock;

} // namespace test

// Main
int main () {
    test::TestCorrectness();
    test::TestSpeed();
    test::TestMultipleManagers();

    return test::s_errorCount;
}
