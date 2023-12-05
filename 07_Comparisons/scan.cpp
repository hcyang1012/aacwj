#include "scan.hpp"
// Standard includes
// C++ Standard
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cctype>
// C Standard

namespace my_cpp {
Scanner::Scanner(std::istream &input) : input_(input), put_back_(kNullChar), line_(1) {
}

Token &Scanner::Curent() const {
    return *current_;
}

size_t Scanner::GetLine() const {
    return line_;
}

void Scanner::Scan() {
    char c = skip();
    switch (c) {
    case EOF:
        current_ = std::make_unique<Token>(Token::Type::T_EOF, "");
        break;
    case '+':
        current_ = std::make_unique<Token>(Token::Type::T_PLUS, std::to_string(c));
        break;
    case '-':
        current_ = std::make_unique<Token>(Token::Type::T_MINUS, std::to_string(c));
        break;
    case '*':
        current_ = std::make_unique<Token>(Token::Type::T_STAR, std::to_string(c));
        break;
    case '/':
        current_ = std::make_unique<Token>(Token::Type::T_SLASH, std::to_string(c));
        break;
    case ';':
        current_ = std::make_unique<Token>(Token::Type::T_SEMI, std::to_string(c));
        break;
    case '=':
        if ((c = next_token()) == '=') {
            current_ = std::make_unique<Token>(Token::Type::T_EQ, "==");
        } else {
            set_put_back(c);
            current_ = std::make_unique<Token>(Token::Type::T_ASSIGN, std::to_string(c));
        }
        break;
    case '!':
        if ((c = next_token()) == '=') {
            current_ = std::make_unique<Token>(Token::Type::T_NE, "!=");
        } else {
            std::stringstream ss;
            ss << "Invalid character : " << c;
            throw std::runtime_error(ss.str());
        }
        break;
    case '<':
        if ((c = next_token()) == '=') {
            current_ = std::make_unique<Token>(Token::Type::T_LE, "<=");
        } else {
            set_put_back(c);
            current_ = std::make_unique<Token>(Token::Type::T_LT, "<");
        }
        break;
    case '>':
        if ((c = next_token()) == '=') {
            current_ = std::make_unique<Token>(Token::Type::T_GE, ">=");
        } else {
            set_put_back(c);
            current_ = std::make_unique<Token>(Token::Type::T_GT, ">");
        }
        break;
    default:
        if (is_digit(c)) {
            int n = scan_int(c);
            current_ = std::make_unique<Token>(
                    Token::Type::T_INTLIT, std::to_string(n), std::make_unique<Value>(Value{n}));
        } else if (std::isalpha(c) || '_' == c) {
            std::string s = scan_id(c);
            try {
                Token::Type type = keyword(s);
                current_ = std::make_unique<Token>(type, s);
            } catch (std::runtime_error &e) {
                current_ = std::make_unique<Token>(Token::Type::T_IDENT, s);
            }

        } else {
            std::stringstream ss;
            ss << "Invalid character : " << c;
            throw std::runtime_error(ss.str());
        }
        break;
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

Token::Type Scanner::keyword(const std::string &s) const {
    auto it = keywords_.find(s);
    if (it != keywords_.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown keyword : " + s);
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

std::string Scanner::scan_id(char c) {
    std::string s;
    while (std::isalpha(c) || isdigit(c) || '_' == c) {
        s += c;
        c = next_token();
    }
    set_put_back(c);
    return s;
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
