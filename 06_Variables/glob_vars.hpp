#pragma once
#include "symbols.hpp"
// Standard includes
// C++ Standard

// C Standard
#include <cstddef>

namespace my_cpp {
size_t find_global_symbol(const std::string &name);
size_t add_global_symbol(const std::string &name);
}