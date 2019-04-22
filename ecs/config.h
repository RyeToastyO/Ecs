namespace ecs {

// This can be large during development.
// Once you actually know your shipping component count,
// this should be set to it to speed up ComponentFlags
#ifndef ECS_MAX_COMPONENTS
#define ECS_MAX_COMPONENTS 256
#endif

#ifndef ECS_MAX_THREADS
#define ECS_MAX_THREADS 4
#endif

#ifndef ECS_TIMESTEP_TYPE
#define ECS_TIMESTEP_TYPE float
typedef ECS_TIMESTEP_TYPE Timestep;
#endif

}
