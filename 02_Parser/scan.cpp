#include "scan.hpp"
// Standard includes
// C++ Standard
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sstream>
// C Standard

namespace my_cpp {
Scanner::Scanner(std::istream &input) : input_(input), put_back_(kNullChar), line_(1) {
}

Token &Scanner::Curent() const {
    return *current_;
}

void Scanner::Scan() {
    char c = skip();
    switch (c) {
    case EOF:
        current_ = std::make_unique<Token>(Token::Type::T_EOF);
        break;
    case '+':
        current_ = std::make_unique<Token>(Token::Type::T_PLUS);
        break;
    case '-':
        current_ = std::make_unique<Token>(Token::Type::T_MINUS);
        break;
    case '*':
        current_ = std::make_unique<Token>(Token::Type::T_STAR);
        break;
    case '/':
        current_ = std::make_unique<Token>(Token::Type::T_SLASH);
        break;
    default:
        if (is_digit(c)) {
            int n = scan_int(c);
            current_ = std::make_unique<Token>(
                    Token::Type::T_INTLIT, std::make_unique<Value>(Value{n}));
        } else {
            std::stringstream ss;
            ss << "Invalid character : " << c;
            throw std::runtime_error(ss.str());
        }
    }
}

void Scanner::set_put_back(const char c) {
    put_back_ = c;
}

char Scanner::get_put_back() const {
    return put_back_;
}

char Scanner::next_token() {
    char c;
    if (get_put_back() != kNullChar) {
        c = get_put_back();
        set_put_back(kNullChar);
        return c;
    }
    c = input_.get();
    if (c == '\n') {
        line_++;
    }
    return c;
}

char Scanner::skip() {
    char c = next_token();
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        c = next_token();
    }
    return c;
}

bool Scanner::is_digit(const char c) {
    return c >= '0' && c <= '9';
}

int Scanner::scan_int(char c) {
    int n = 0;
    while (is_digit(c)) {
        n = 10 * n + (c - '0');
        c = next_token();
    }
    set_put_back(c);
    return n;
}

namespace utility {
std::unique_ptr<Scanner> MakeScanner(std::istream &input) {
    return std::make_unique<Scanner>(Scanner(input));
}

void ScanFile(const std::string &filename) {
    std::ifstream input(filename);
    if (!input) {
        throw std::runtime_error("Cannot open file");
    }
    auto scanner = MakeScanner(input);
    scanner->Scan();
    const auto &token = scanner->Curent();
    while (token.GetType() != Token::Type::T_EOF) {
        std::cout << token << std::endl;
        scanner->Scan();
    }
}
}
}
