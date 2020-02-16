/*
 * Copyright (c) 2020 Riley Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include <iostream>
#include <mutex>

namespace test {

// Helpers
#define EXPECT_TRUE(condition)                                                                  \
    if (!(condition)) {                                                                         \
        ::test::s_errorLock.lock();                                                             \
        ++::test::s_errorCount;                                                                 \
        std::cout << __FUNCTION__ << "(line " << __LINE__ << "): " << #condition << std::endl;  \
        ::test::s_errorLock.unlock();                                                           \
    }
#define EXPECT_FALSE(condition) EXPECT_TRUE(!(condition));

// Static data
extern int s_errorCount;
extern std::mutex s_errorLock;

}
