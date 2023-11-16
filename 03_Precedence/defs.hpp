#pragma once

// Standard includes
// C++ Standard
#include <memory>
#include <ostream>
// C Standard
#include <cstddef>

namespace my_cpp {
union Value {
    int int_value_;
};
// Token class
class Token {
public:
    // Types of tokens
    enum Type {
        T_EOF = 0,
        T_PLUS,
        T_MINUS,
        T_STAR,
        T_SLASH,
        T_INTLIT,
    };

    Token(Type type, std::unique_ptr<Value> value = nullptr);
    ~Token() = default;
    Type GetType() const;
    template <typename T>
    T GetValue() const;

private:
    Type type_;
    std::unique_ptr<Value> value_;
};

}
std::ostream &operator<<(std::ostream &os, const my_cpp::Token &token);