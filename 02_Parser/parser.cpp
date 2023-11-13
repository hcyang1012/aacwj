#include "parser.hpp"
#include "defs.hpp"
// Standard includes
// C++ Standard
#include <memory>
// C Standard

namespace my_cpp {
Parser::Parser(std::unique_ptr<Scanner> scanner) : scanner_(std::move(scanner)) {
}

std::shared_ptr<ASTNode> Parser::Parse() {
    return bin_expr();
}

std::shared_ptr<ASTNode> Parser::primary() {
    const auto &token = scanner_->Curent();
    std::shared_ptr<ASTNode> result = nullptr;
    switch (token.GetType()) {
    case Token::Type::T_INTLIT: {
        auto value = std::make_unique<Value>();
        value->int_value_ = token.GetValue<int>();
        result = ASTNode::MakeAstLeaf(ASTNode::Type::A_INTLIT, std::move(value));
        scanner_->Scan();
    } break;
    default:
        throw std::runtime_error("Invalid token");
    }
    return result;
}

std::shared_ptr<ASTNode> Parser::bin_expr() {
    std::shared_ptr<ASTNode> result, left, right;
    left = primary();

    if (scanner_->Curent().GetType() == Token::Type::T_EOF) {
        return left;
    }

    const auto &token = arith_op(scanner_->Curent().GetType());
    scanner_->Scan();
    right = bin_expr();
    result = ASTNode::MakeAstNode(token, left, right);
    return result;
}

ASTNode::Type Parser::arith_op(const Token::Type &type) const {
    switch (type) {
    case Token::Type::T_PLUS:
        return ASTNode::Type::A_ADD;
    case Token::Type::T_MINUS:
        return ASTNode::Type::A_SUBTRACT;
    case Token::Type::T_STAR:
        return ASTNode::Type::A_MULTIPLY;
    case Token::Type::T_SLASH:
        return ASTNode::Type::A_DIVIDE;
    default:
        throw std::runtime_error("Invalid token");
    }
};

}