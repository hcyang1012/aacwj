#include "parser.hpp"

#include "defs.hpp"
#include "glob_vars.hpp"
// Standard includes
// C++ Standard
#include <memory>
#include <sstream>
// C Standard

namespace my_cpp {
Parser::Parser(std::unique_ptr<Scanner> scanner) : scanner_(std::move(scanner)) {
}

std::shared_ptr<ASTNode> Parser::Parse() {
    return compound_stmts();
}

std::shared_ptr<ASTNode> Parser::primary() {
    const auto &token = scanner_->Curent();
    std::shared_ptr<ASTNode> result = nullptr;
    switch (token.GetType()) {
    case Token::Type::T_INTLIT: {
        auto value = std::make_unique<Value>();
        value->int_value_ = token.GetValue<int>();
        result = ASTNode::MakeAstLeaf(ASTNode::Type::A_INTLIT, std::move(value));
    } break;
    case Token::Type::T_IDENT: {
        auto global_symbol = find_global_symbol(token.GetText());
        auto value = std::make_unique<Value>();
        value->sym_id = global_symbol;
        result = ASTNode::MakeAstLeaf(ASTNode::Type::A_IDENT, std::move(value));
    } break;
    default:
        throw std::runtime_error("Invalid token");
    }
    scanner_->Scan();
    return result;
}

std::shared_ptr<ASTNode> Parser::bin_expr(const size_t prev_precedence) {
    std::shared_ptr<ASTNode> left, right;
    left = primary();

    if (scanner_->Curent().GetType() == Token::Type::T_SEMI
        || scanner_->Curent().GetType() == Token::Type::T_RPAREN) {
        return left;
    }
    auto current_op_type = scanner_->Curent().GetType();
    auto current_op_precedence = get_priority(scanner_->Curent());
    while (current_op_precedence > prev_precedence) {
        scanner_->Scan();
        right = bin_expr(current_op_precedence);
        left = ASTNode::MakeAstNode(arith_op(current_op_type), left, nullptr, right);
        auto token = scanner_->Curent().GetType();
        current_op_type = scanner_->Curent().GetType();
        if (current_op_type == Token::Type::T_SEMI || current_op_type == Token::Type::T_RPAREN) {
            break;
        }
        current_op_precedence = get_priority(scanner_->Curent());
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::compound_stmts() {
    lbrace();
    std::shared_ptr<ASTNode> left, tree;
    while (true) {
        switch (scanner_->Curent().GetType()) {
        case Token::Type::T_PRINT:
            tree = print_stmt();
            break;
        case Token::Type::T_INT:
            tree = var_decl_stmt();
            break;
        case Token::Type::T_IDENT:
            tree = assign_stmt();
            break;
        case Token::Type::T_IF:
            tree = if_stmt();
            break;
        case Token::Type::T_RBRACE:
            rbrace();
            return left;
        default:
            throw std::runtime_error("Invalid token");
        }
        if (tree != nullptr) {
            if (left == nullptr) {
                left = tree;
            } else {
                left = ASTNode::MakeAstNode(ASTNode::Type::A_GLUE, left, nullptr, tree);
            }
        }
    }
}

std::shared_ptr<ASTNode> Parser::if_stmt() {
    std::shared_ptr<ASTNode> cond, true_stmt, false_stmt;

    match(Token::Type::T_IF);
    lparen();
    cond = bin_expr();

    if (!(ASTNode::Type::A_EQ <= cond->GetOp() && cond->GetOp() <= ASTNode::Type::A_GE)) {
        throw std::runtime_error("Invalid comparison operator");
    }
    rparen();

    true_stmt = compound_stmts();

    if (scanner_->Curent().GetType() == Token::Type::T_ELSE) {
        scanner_->Scan();
        false_stmt = compound_stmts();
    }

    return ASTNode::MakeAstNode(ASTNode::Type::A_IF, cond, true_stmt, false_stmt);
}

ASTNode::Type Parser::arith_op(const Token::Type &type) const {
    if (Token::Type::T_EOF < type && type < Token::Type::T_INTLIT) {
        return static_cast<ASTNode::Type>(type);
    } else {
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

void Parser::match(Token::Type type) {
    if (scanner_->Curent().GetType() == type) {
        scanner_->Scan();
    } else {
        std::stringstream ss;
        ss << type << " expected at line " << scanner_->GetLine();
        throw std::runtime_error(ss.str());
    }
}

void Parser::semi() {
    match(Token::Type::T_SEMI);
}

void Parser::ident() {
    match(Token::Type::T_IDENT);
}

void Parser::lbrace() {
    match(Token::Type::T_LBRACE);
}

void Parser::rbrace() {
    match(Token::Type::T_RBRACE);
}

void Parser::lparen() {
    match(Token::Type::T_LPAREN);
}

void Parser::rparen() {
    match(Token::Type::T_RPAREN);
}

std::shared_ptr<ASTNode> Parser::print_stmt() {
    match(Token::Type::T_PRINT);
    auto tree = bin_expr();
    tree = ASTNode::MakeAstUnary(ASTNode::Type::A_PRINT, tree);
    semi();
    return tree;
}

std::shared_ptr<ASTNode> Parser::assign_stmt() {
    const auto kIdentName = scanner_->Curent().GetText();
    ident();
    auto global_symbol = find_global_symbol(kIdentName);
    auto sym_id = std::make_unique<Value>();
    sym_id->sym_id = global_symbol;
    auto right = ASTNode::MakeAstLeaf(ASTNode::Type::A_LVIDENT, std::move(sym_id));

    match(Token::Type::T_ASSIGN);

    auto left = bin_expr();

    auto tree = ASTNode::MakeAstNode(ASTNode::Type::A_ASSIGN, left, nullptr, right);
    semi();

    return tree;
}

std::shared_ptr<ASTNode> Parser::var_decl_stmt() {
    match(Token::Type::T_INT);
    const auto kIdentName = scanner_->Curent().GetText();
    ident();
    add_global_symbol(kIdentName);
    semi();
    auto value = std::make_unique<Value>();
    value->sym_id = find_global_symbol(kIdentName);
    auto left = ASTNode::MakeAstLeaf(ASTNode::Type::A_VAR_DECL, std::move(value));
    return left;
}
}  // namespace my_cpp