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
    ECS_RUN_THIS_BEFORE(Job4);

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
    ecs::impl::JobTree * tree = ecs::impl::JobTree::New<JobDataSortingGroup>();

    // TODO: Figure out how to validate this tree automatically

    delete tree;
}

// Explicit Sort
void TestExplicitSorting () {
    // TODO:
}

struct CycleA { float Value; };
struct CycleB { float Value; };
struct CycleC { float Value; };

struct CycleJob1 : ecs::Job {
    ECS_READ(CycleA, A);
    ECS_WRITE(CycleB, B);
};

struct CycleJob2 : ecs::Job {
    ECS_READ(CycleB, B);
    ECS_WRITE(CycleC, C);
};

struct CycleJob3 : ecs::Job {
    ECS_READ(CycleC, C);
    ECS_WRITE(CycleA, A);
};

struct CycleD { float Value; };

struct CycleJob4 : ecs::Job {
    ECS_READ(CycleA, A);
    ECS_READ(CycleB, B);
    ECS_READ(CycleC, C);

    ECS_WRITE(CycleD, D);
};

struct CycleJob5 : ecs::Job {
    ECS_READ(CycleD, D);
};

struct CycleJob6 : ecs::Job {
    ECS_READ(CycleD, D);
};

struct CycleJob7 : ecs::Job {
    ECS_READ(CycleD, D);
};

struct CycleUpdateGroup : ecs::IUpdateGroup {};

ECS_REGISTER_JOB_FOR_UPDATE_GROUP(CycleJob7, CycleUpdateGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(CycleJob6, CycleUpdateGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(CycleJob5, CycleUpdateGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(CycleJob4, CycleUpdateGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(CycleJob1, CycleUpdateGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(CycleJob2, CycleUpdateGroup);
ECS_REGISTER_JOB_FOR_UPDATE_GROUP(CycleJob3, CycleUpdateGroup);

void TestDataCycle () {
    ecs::impl::JobTree * tree = ecs::impl::JobTree::New<CycleUpdateGroup>();

    // TODO: figure out how to validate this tree automatically
    // It is currently wrong

    delete tree;
}

void TestJobSorting () {
    TestDataSorting();
    TestExplicitSorting();
    TestDataCycle();
}

} // namespace test
