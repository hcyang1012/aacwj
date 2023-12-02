#include "symbols.hpp"

// Standard includes
// C++ Standard
#include <stdexcept>
// C Standard

namespace my_cpp {

SymbolTableEntry::SymbolTableEntry(const std::string &name) : name_(name) {
}

const std::string &SymbolTableEntry::GetName() const {
    return name_;
}
}
