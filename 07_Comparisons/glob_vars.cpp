#include "glob_vars.hpp"
// Standard includes
#include <vector>
#include <algorithm>
// C++ Standard
#include <sstream>
// C Standard

namespace my_cpp {
std::vector<SymbolTableEntry> global_symbol_table;
size_t find_global_symbol(const std::string &name) {
    auto it = std::find_if(
            global_symbol_table.begin(),
            global_symbol_table.end(),
            [&name](const SymbolTableEntry &entry) { return entry.GetName() == name; });
    if (it == global_symbol_table.end()) {
        throw std::runtime_error("Symbol not found");
    }
    return std::distance(global_symbol_table.begin(), it);
}
size_t add_global_symbol(const std::string &name) {
    try {
        find_global_symbol(name);
    } catch (std::runtime_error &e) {
        global_symbol_table.emplace_back(name);
    }
    return find_global_symbol(name);
}
}