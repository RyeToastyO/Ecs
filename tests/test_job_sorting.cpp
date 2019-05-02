/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#include "tests.h"

#include "../ecs/ecs.h"

namespace test {

struct CA { float Value; };
struct CB { float Value; };
struct CC { float Value; };
struct CD { float Value; };
struct CE { float Value; };
struct CF { float Value; };

struct Job1 : ecs::Job {
    ECS_WRITE(CA, A);
    ECS_WRITE(CB, B);

    ECS_READ(CC, C);
    ECS_READ(CD, D);
};

struct Job2 : ecs::Job {
    ECS_WRITE(CA, A);
    ECS_WRITE(CC, C);

    ECS_READ(CB, B);
};

struct Job3 : ecs::Job {
    ECS_READ(CC, C);
    ECS_READ(CD, D);
};

struct Job4 : ecs::Job {
    ECS_WRITE(CF, F);

    ECS_READ(CD, D);
};

struct Job5 : ecs::Job {
    ECS_WRITE(CB, B);

    ECS_READ(CA, A);
};

struct Job6 : ecs::Job {
    ECS_READ(CC, C);
    ECS_READ(CD, D);
};

struct Job7 : ecs::Job {
    ECS_WRITE(CE, E);
    ECS_WRITE(CF, F);

    ECS_READ(CA, A);
};

struct Job8 : ecs::Job {
    ECS_READ(CE, E);
};

struct Job9 : ecs::Job {
    ECS_READ(CF, F);
};

struct Job10 : ecs::Job {
    ECS_READ(CE, E);
    ECS_READ(CF, F);
};

struct JobDataSortingGroup : ecs::IUpdateGroup {};

ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job10, JobDataSortingGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job9, JobDataSortingGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job8, JobDataSortingGroup);

ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job3, JobDataSortingGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job2, JobDataSortingGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job1, JobDataSortingGroup);

ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job7, JobDataSortingGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job6, JobDataSortingGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job5, JobDataSortingGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(Job4, JobDataSortingGroup);

void TestDataSorting () {
    //ecs::impl::JobTree * tree = ecs::impl::JobTree::Create<JobDataSortingGroup>();

    //tree->ForEachNode([](ecs::impl::JobNode * node) {
    //    std::cout << node->dependents.size() << std::endl;
    //});
}

// Explicit Sort

void TestExplicitSorting () {
    //ecs::Manager mgr;
}

void TestJobSorting () {
    TestDataSorting();
    TestExplicitSorting();
}

} // namespace test
