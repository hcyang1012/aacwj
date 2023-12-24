#pragma once

// Project includes
#include "defs.hpp"

// Standard includes
// C++ Standard
#include <istream>
#include <map>
#include <memory>

// C Standard
#include <cstddef>
#include <cstdio>

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
      {"print", Token::Type::T_PRINT}, {"int", Token::Type::T_INT},
      {"if", Token::Type::T_IF},       {"else", Token::Type::T_ELSE},
      {"while", Token::Type::T_WHILE},
  };
};

namespace utility {
std::unique_ptr<Scanner> MakeScanner(std::istream &input);

void ScanFile(const std::string &filename);
}  // namespace utility
}  // namespace my_cpp