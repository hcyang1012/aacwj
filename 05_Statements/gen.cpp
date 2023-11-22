#include "gen.hpp"
// Standard includes
// C++ Standard
// C Standard

namespace my_cpp {
CodeGenerator::CodeGenerator(std::ostream &os) : os_(os) {
}
void CodeGenerator::GenerateCode(const std::vector<std::shared_ptr<ASTNode>> &stmts) {
    codegen_preemble();
    for (const auto &stmt : stmts) {
        size_t reg = codegen_ast(*stmt);
        codegen_printint(reg);
        registers_free_all();
    }

    codegen_postemble();
}

size_t CodeGenerator::codegen_ast(const ASTNode &node) {
    size_t left_reg, right_reg;

    if (node.GetLeft() != nullptr) {
        left_reg = codegen_ast(*node.GetLeft());
    }
    if (node.GetRight() != nullptr) {
        right_reg = codegen_ast(*node.GetRight());
    }

    switch (node.GetOp()) {
    case ASTNode::Type::A_ADD:
        return codegen_add(left_reg, right_reg);
    case ASTNode::Type::A_SUBTRACT:
        return codegen_sub(left_reg, right_reg);
    case ASTNode::Type::A_MULTIPLY:
        return codegen_mul(left_reg, right_reg);
    case ASTNode::Type::A_DIVIDE:
        return codegen_div(left_reg, right_reg);
    case ASTNode::Type::A_INTLIT:
        return codegen_load(node.GetValue<int>());
    default:
        throw std::runtime_error("Invalid ASTNode type");
    }
}
}