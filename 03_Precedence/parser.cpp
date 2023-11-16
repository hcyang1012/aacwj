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

std::shared_ptr<ASTNode> Parser::bin_expr(const size_t prev_precedence) {
    std::shared_ptr<ASTNode> left, right;
    left = primary();

    if (scanner_->Curent().GetType() == Token::Type::T_EOF) {
        return left;
    }
    auto current_op_type = scanner_->Curent().GetType();
    auto current_op_precedence = get_priority(scanner_->Curent());
    while (current_op_precedence > prev_precedence) {
        scanner_->Scan();
        right = bin_expr(current_op_precedence);
        left = ASTNode::MakeAstNode(arith_op(current_op_type), left, right);
        auto token = scanner_->Curent().GetType();
        if (token == Token::Type::T_EOF) {
            break;
        }
        current_op_type = scanner_->Curent().GetType();
        current_op_precedence = get_priority(scanner_->Curent());
    }
    return left;
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

size_t Parser::get_priority(const Token &token) const {
    if (token.GetType() >= sizeof(kPriority) / sizeof(kPriority[0])) {
        throw std::runtime_error("Invalid token");
    }
    auto prec = kPriority[static_cast<size_t>(token.GetType())];
    if (prec == 0) {
        throw std::runtime_error("Invalid token at line " + std::to_string(scanner_->GetLine()));
    }
    return prec;
}
}