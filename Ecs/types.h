#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

typedef uint8_t byte_t;

#define ARRAY(type) ::std::vector<type>
#define DICTIONARY(key, value) ::std::unordered_map<key, value>
