#include "defs.hpp"

// Standard includes
// C++ Standard
#include <stdexcept>
// C Standard

namespace my_cpp {

  Token::Token(Type type, std::unique_ptr<Value> value) : type_(type), value_(std::move(value)) {
  }

  Token::Type Token::GetType() const {
    return type_;
  }

  template <typename T>
  T Token::GetValue() const {
    if (value_ == nullptr) {
      throw std::runtime_error("Token has no value");
    }
    switch (type_) {
    case Type::T_INTLIT:
      return static_cast<T>(value_->int_value_);
    default:
      throw std::runtime_error("Invalid token type");
    }
  }
}

std::ostream& operator<<(std::ostream& os, const my_cpp::Token& token) {
  switch (token.GetType()) {
  case my_cpp::Token::Type::T_EOF:
    os << "TOKEN EOF";
    break;
  case my_cpp::Token::Type::T_PLUS:
    os << "TOKEN +";
    break;
  case my_cpp::Token::Type::T_MINUS:
    os << "TOKEN -";
    break;
  case my_cpp::Token::Type::T_STAR:
    os << "TOKEN *";
    break;
  case my_cpp::Token::Type::T_SLASH:
    os << "TOKEN /";
    break;
  case my_cpp::Token::Type::T_INTLIT:
    os << "TOKEN INTLIT(" << token.GetValue<int>() << ")";
    break;
  default:
    throw std::runtime_error("Invalid token type");
  }
  return os;
}