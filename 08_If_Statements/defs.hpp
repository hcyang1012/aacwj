#pragma once

// Standard includes
// C++ Standard
#include <memory>
#include <ostream>
#include <string>
// C Standard
#include <cstddef>

namespace my_cpp {
union Value {
    int int_value_;
    size_t sym_id;
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
        T_EQ,
        T_NE,
        T_LT,
        T_GT,
        T_LE,
        T_GE,
        T_INTLIT,
        T_SEMI,
        T_ASSIGN,
        T_IDENT,
        T_LBRACE,
        T_RBRACE,
        T_LPAREN,
        T_RPAREN,

        // Keywords
        T_PRINT,
        T_INT,
        T_IF,
        T_ELSE,
    };

    Token(Type type, const std::string &text, std::unique_ptr<Value> value = nullptr);
    ~Token() = default;
    Type GetType() const;
    std::string GetText() const;
    template <typename T>
    T GetValue() const;

private:
    Type type_;
    std::unique_ptr<Value> value_;
    std::string text_;
};

}
std::ostream &operator<<(std::ostream &os, const my_cpp::Token &token);