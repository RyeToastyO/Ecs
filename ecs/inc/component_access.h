/*
 * Copyright (c) 2019 Ryan Diederich
 * License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
 */

#pragma once

#include "../helpers/token_combine.h"

namespace ecs {

// Component Access macros
#define ECS_EXCLUDE(...) ::ecs::impl::Exclude<__VA_ARGS__> ECS_TOKEN_COMBINE(__exclude, __LINE__) = ::ecs::impl::Exclude<__VA_ARGS__>(*this);

#define ECS_READ(componentType, variableName)                                               \
::ecs::impl::Read<componentType> variableName = ::ecs::impl::Read<componentType>(*this);    \
static_assert(!std::is_empty<componentType>(), "Cannot read access an empty/tag component, use ECS_REQUIRE");

#define ECS_READ_OTHER(componentType, variableName)                                                                         \
::ecs::impl::ReadOther<componentType> variableName = ::ecs::impl::ReadOther<componentType>(*this);                          \
static_assert(!std::is_empty<componentType>(), "Cannot read access an empty/tag component, use HasComponent<T>(entity)");   \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "ReadOther Entity doesn't even make sense");

#define ECS_READ_SINGLETON(componentType, variableName)                                                     \
::ecs::impl::ReadSingleton<componentType> variableName = ::ecs::impl::ReadSingleton<componentType>(*this);  \
static_assert(std::is_base_of<::ecs::ISingletonComponent, componentType>::value, "Must inherit ISingletonComponent to be read in this way");

#define ECS_REQUIRE(...) ::ecs::impl::Require<__VA_ARGS__> ECS_TOKEN_COMBINE(__require, __LINE__) = ::ecs::impl::Require<__VA_ARGS__>(*this);
#define ECS_REQUIRE_ANY(...) ::ecs::impl::RequireAny<__VA_ARGS__> ECS_TOKEN_COMBINE(__requireAny, __LINE__) = ::ecs::impl::RequireAny<__VA_ARGS__>(*this);

#define ECS_WRITE(componentType, variableName)                                                                  \
::ecs::impl::Write<componentType> variableName = ::ecs::impl::Write<componentType>(*this);                      \
static_assert(!std::is_empty<componentType>(), "Cannot write access an empty/tag component, use ECS_REQUIRE");  \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "Don't write to Entity, you will break everything");

#define ECS_WRITE_OTHER(componentType, variableName)                                                                        \
::ecs::impl::WriteOther<componentType> variableName = ::ecs::impl::WriteOther<componentType>(*this);                        \
static_assert(!std::is_empty<componentType>(), "Cannot write access an empty/tag component, use HasComponent<T>(entity)");  \
static_assert(!std::is_same<std::remove_const<componentType>::type, ::ecs::Entity>::value, "Don't write to Entity, you will break everything");

#define ECS_WRITE_SINGLETON(componentType, variableName)                                                        \
::ecs::impl::WriteSingleton<componentType> variableName = ::ecs::impl::WriteSingleton<componentType>(*this);    \
static_assert(std::is_base_of<::ecs::ISingletonComponent, componentType>::value, "Must inherit ISingletonComponent to be written in this way");

} // namespace ecs
