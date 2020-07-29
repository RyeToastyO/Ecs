// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#ifdef _DEBUG
#define TEST_MEMORY_LEAKS
#endif

#ifdef TEST_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include "test_correctness.h"
#include "test_speed.h"
#include <mutex>

namespace test {

// Static data
int s_errorCount = 0;
std::mutex s_errorLock;

} // namespace test

// Main
int main () {
#ifdef TEST_MEMORY_LEAKS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    test::TestCorrectness();
    test::TestSpeed();

    return test::s_errorCount;
}
