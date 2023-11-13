#pragma once
#include "defs.hpp"
#include "ast.hpp"
#include "scan.hpp"
// Standard includes
// C++ Standard
#include <memory>
#include <ostream>
// C Standard
#include <cstddef>

namespace my_cpp {
class Parser {
public:
    Parser(std::unique_ptr<Scanner> scanner);
    std::shared_ptr<ASTNode> Parse();

private:
    std::unique_ptr<Scanner> scanner_;

    std::shared_ptr<ASTNode> primary();
    std::shared_ptr<ASTNode> bin_expr();

    ASTNode::Type arith_op(const Token::Type &type) const;
};
}