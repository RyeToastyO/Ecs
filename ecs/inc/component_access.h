/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "../helpers/token_combine.h"

namespace ecs {

// Component Access macros
#define ECS_EXCLUDE(...) Exclude<__VA_ARGS__> ECS_TOKEN_COMBINE(__exclude, __LINE__) = Exclude<__VA_ARGS__>(*this);

#define ECS_READ(componentType, variableName)                   \
Read<componentType> variableName = Read<componentType>(*this);  \
static_assert(!std::is_empty<componentType>(), "Cannot read access an empty/tag component, use ECS_REQUIRE");

#define ECS_READ_OTHER(componentType, variableName)                                                                         \
ReadOther<componentType> variableName = ReadOther<componentType>(*this);                                                    \
static_assert(!std::is_empty<componentType>(), "Cannot read access an empty/tag component, use HasComponent<T>(entity)");   \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "ReadOther Entity doesn't even make sense");

#define ECS_READ_SINGLETON(componentType, variableName)                             \
ReadSingleton<componentType> variableName = ReadSingleton<componentType>(*this);    \
static_assert(std::is_base_of<ISingletonComponent, componentType>::value, "Must inherit ISingletonComponent to be read in this way");

#define ECS_REQUIRE(...) Require<__VA_ARGS__> ECS_TOKEN_COMBINE(__require, __LINE__) = Require<__VA_ARGS__>(*this);
#define ECS_REQUIRE_ANY(...) RequireAny<__VA_ARGS__> ECS_TOKEN_COMBINE(__requireAny, __LINE__) = RequireAny<__VA_ARGS__>(*this);

#define ECS_WRITE(componentType, variableName)                                                                  \
Write<componentType> variableName = Write<componentType>(*this);                                                \
static_assert(!std::is_empty<componentType>(), "Cannot write access an empty/tag component, use ECS_REQUIRE");  \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "Don't write to Entity, you will break everything");

#define ECS_WRITE_OTHER(componentType, variableName)                                                                        \
WriteOther<componentType> variableName = WriteOther<componentType>(*this);                                                  \
static_assert(!std::is_empty<componentType>(), "Cannot write access an empty/tag component, use HasComponent<T>(entity)");  \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "Don't write to Entity, you will break everything");

#define ECS_WRITE_SINGLETON(componentType, variableName)                            \
WriteSingleton<componentType> variableName = WriteSingleton<componentType>(*this);  \
static_assert(std::is_base_of<ISingletonComponent, componentType>::value, "Must inherit ISingletonComponent to be written in this way");

}

#include "inline/component_access.inl"
