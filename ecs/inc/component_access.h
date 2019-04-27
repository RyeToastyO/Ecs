/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "../helpers/token_combine.h"

namespace ecs {

// Component Access macros
#define ECS_EXCLUDE(...) ::ecs::Exclude<__VA_ARGS__> ECS_TOKEN_COMBINE(__exclude, __LINE__) = ::ecs::Exclude<__VA_ARGS__>(*this);

#define ECS_READ(componentType, variableName)                                   \
::ecs::Read<componentType> variableName = ::ecs::Read<componentType>(*this);    \
static_assert(!std::is_empty<componentType>(), "Cannot read access an empty/tag component, use ECS_REQUIRE");

#define ECS_READ_OTHER(componentType, variableName)                                                                         \
::ecs::ReadOther<componentType> variableName = ::ecs::ReadOther<componentType>(*this);                                      \
static_assert(!std::is_empty<componentType>(), "Cannot read access an empty/tag component, use HasComponent<T>(entity)");   \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "ReadOther Entity doesn't even make sense");

#define ECS_READ_SINGLETON(componentType, variableName)                                         \
::ecs::ReadSingleton<componentType> variableName = ::ecs::ReadSingleton<componentType>(*this);  \
static_assert(std::is_base_of<::ecs::ISingletonComponent, componentType>::value, "Must inherit ISingletonComponent to be read in this way");

#define ECS_REQUIRE(...) ::ecs::Require<__VA_ARGS__> ECS_TOKEN_COMBINE(__require, __LINE__) = ::ecs::Require<__VA_ARGS__>(*this);
#define ECS_REQUIRE_ANY(...) ::ecs::RequireAny<__VA_ARGS__> ECS_TOKEN_COMBINE(__requireAny, __LINE__) = ::ecs::RequireAny<__VA_ARGS__>(*this);

#define ECS_WRITE(componentType, variableName)                                                                  \
::ecs::Write<componentType> variableName = ::ecs::Write<componentType>(*this);                                  \
static_assert(!std::is_empty<componentType>(), "Cannot write access an empty/tag component, use ECS_REQUIRE");  \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "Don't write to Entity, you will break everything");

#define ECS_WRITE_OTHER(componentType, variableName)                                                                        \
::ecs::WriteOther<componentType> variableName = ::ecs::WriteOther<componentType>(*this);                                    \
static_assert(!std::is_empty<componentType>(), "Cannot write access an empty/tag component, use HasComponent<T>(entity)");  \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "Don't write to Entity, you will break everything");

#define ECS_WRITE_SINGLETON(componentType, variableName)                                            \
::ecs::WriteSingleton<componentType> variableName = ::ecs::WriteSingleton<componentType>(*this);    \
static_assert(std::is_base_of<::ecs::ISingletonComponent, componentType>::value, "Must inherit ISingletonComponent to be written in this way");

} // namespace ecs
