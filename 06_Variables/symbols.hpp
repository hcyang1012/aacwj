#pragma once

// Standard includes
// C++ Standard
#include <string>
// C Standard
#include <cstddef>

namespace my_cpp {

class SymbolTableEntry {
public:
    SymbolTableEntry(const std::string &name);
    ~SymbolTableEntry() = default;
    const std::string &GetName() const;

private:
    std::string name_;
};

}