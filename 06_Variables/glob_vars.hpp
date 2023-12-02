#pragma once
#include "symbols.hpp"
// Standard includes
// C++ Standard

// C Standard
#include <cstddef>
#include <vector>

namespace my_cpp {
extern std::vector<SymbolTableEntry> global_symbol_table;
size_t find_global_symbol(const std::string &name);
size_t add_global_symbol(const std::string &name);
}  // namespace my_cpp