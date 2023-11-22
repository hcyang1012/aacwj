#pragma once

// Project includes
#include "defs.hpp"

// Standard includes
// C++ Standard
#include <istream>
#include <memory>
#include <map>

// C Standard
#include <cstdio>
#include <cstddef>

namespace my_cpp {
class Scanner {
public:
    Scanner(std::istream &input);

    Token &Curent() const;
    size_t GetLine() const;
    void Scan();

private:
    static constexpr auto kNullChar = '\0';

    std::istream &input_;
    char put_back_;
    size_t line_;
    std::unique_ptr<Token> current_;

    void set_put_back(const char c);
    char get_put_back() const;
    char next_token();
    char skip();
    bool is_digit(const char c);
    Token::Type keyword(const std::string &s) const;

    int scan_int(char c);
    std::string scan_id(char c);

    const std::map<std::string, Token::Type> keywords_ = {
            {"print", Token::Type::T_PRINT},
    };
};

namespace utility {
std::unique_ptr<Scanner> MakeScanner(std::istream &input);

void ScanFile(const std::string &filename);
}
}