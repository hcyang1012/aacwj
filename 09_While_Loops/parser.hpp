#pragma once
#include "ast.hpp"
#include "defs.hpp"
#include "scan.hpp"
// Standard includes
// C++ Standard
#include <memory>
#include <ostream>
#include <vector>
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
    std::shared_ptr<ASTNode> bin_expr(const size_t prev_precedence = 0);
    std::shared_ptr<ASTNode> compound_stmts();
    std::shared_ptr<ASTNode> if_stmt();
    std::shared_ptr<ASTNode> while_stmt();
    ASTNode::Type arith_op(const Token::Type &type) const;
    size_t get_priority(const Token &token) const;
    void match(Token::Token::Type type);
    void semi();
    void ident();
    void lbrace();
    void rbrace();
    void lparen();
    void rparen();

    // Statements
    std::shared_ptr<ASTNode> print_stmt();
    std::shared_ptr<ASTNode> assign_stmt();
    std::shared_ptr<ASTNode> var_decl_stmt();

    static constexpr size_t kPriority[] = {
            0,   // EOF
            10,  // +
            10,  // -
            20,  // *
            20,  // /
            30,  // ==
            30,  // !=
            40,  // <
            40,  // >
            40,  // <=
            40,  // >=
            0,   // INTLIT
    };
};
}  // namespace my_cpp