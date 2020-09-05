// ----------------------------------------------------------------------------
// Copyright (c) 2020 Riley Diederich
// License (MIT): https://github.com/RyeToastyO/Ecs/blob/master/LICENSE
// ----------------------------------------------------------------------------

#pragma once

#include <functional>

namespace ecs {
namespace impl {

// djb2 string hash algorithm
inline uint64_t StringHash (const char* str)
{
  uint64_t hash = 5381;
  int c;
  while ((c = *str++) != 0)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
}

} // namespace impl
} // namespace ecs
