#pragma once

// Project includes
#include "defs.hpp"

// Standard includes
// C++ Standard
#include <istream>
#include <memory>

// C Standard
#include <cstdio>
#include <cstddef>

namespace my_cpp {
  class Scanner {
  public:
    Scanner(std::istream& input);

    std::unique_ptr<Token> Scan();

  private:
    static constexpr auto kNullChar = '\0';

    std::istream& input_;
    char put_back_;
    size_t line_;

    void set_put_back(const char c);
    char get_put_back() const;
    char next_token();
    char skip();
    bool is_digit(const char c);

    int scan_int(char c);

  };

  namespace utility {
    std::unique_ptr<Scanner> MakeScanner(std::istream& input);

    void ScanFile(const std::string& filename);
  }
}