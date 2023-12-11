#include "gen.hpp"

#include "glob_vars.hpp"
// Standard includes
// C++ Standard
// C Standard

namespace my_cpp {
CodeGenerator::CodeGenerator(std::ostream &os) : os_(os) {
}
void CodeGenerator::GenerateCode(const std::shared_ptr<ASTNode> &root) {
    codegen_preemble();
    codegen_ast(*root);
    registers_free_all();

    codegen_postemble();
}

size_t CodeGenerator::codegen_if(const ASTNode &if_stmt) {
    size_t label_false, label_end;
    auto label = [&]() { return label_id++; };
    label_false = label();

    if (if_stmt.GetRight() != nullptr) {
        label_end = label();
    }

    codegen_ast(*(if_stmt.GetLeft()), label_end, if_stmt.GetOp());
    registers_free_all();

    codegen_ast(*(if_stmt.GetMiddle()), std::nullopt, if_stmt.GetOp());

    if (if_stmt.GetRight() != nullptr) {
        codegen_jump(label_end);
    }

    codegen_label(label_false);
    if (if_stmt.GetRight() != nullptr) {
        codegen_ast(*(if_stmt.GetRight()), std::nullopt, if_stmt.GetOp());
        registers_free_all();
        codegen_label(label_end);
    }
    return kNoRegister;
}

size_t CodeGenerator::codegen_ast(
        const ASTNode &node,
        std::optional<size_t> reg,
        const ASTNode::Type parent_op) {
    size_t left_reg, right_reg;

    switch (node.GetOp()) {
    case ASTNode::Type::A_IF:
        return codegen_if(node);
    case ASTNode::Type::A_GLUE:
        if (node.GetLeft() != nullptr) {
            codegen_ast(*node.GetLeft(), std::nullopt, parent_op);
            registers_free_all();
        }
        if (node.GetRight() != nullptr) {
            codegen_ast(*node.GetRight(), std::nullopt, parent_op);
            registers_free_all();
        }
        return kNoRegister;
    }

    if (node.GetLeft() != nullptr) {
        left_reg = codegen_ast(*node.GetLeft());
    }
    if (node.GetRight() != nullptr) {
        right_reg = codegen_ast(*node.GetRight(), left_reg);
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
    case ASTNode::Type::A_EQ:
    case ASTNode::Type::A_NE:
    case ASTNode::Type::A_LT:
    case ASTNode::Type::A_GT:
    case ASTNode::Type::A_LE:
    case ASTNode::Type::A_GE: {
        if (parent_op == ASTNode::Type::A_IF) {
            return codegen_compare_and_jump(node.GetOp(), left_reg, right_reg, *reg);
        } else {
            return codegen_compare_and_set(node.GetOp(), left_reg, right_reg);
        }
    }
    case ASTNode::Type::A_INTLIT:
        return codegen_load_int(node.GetValue<int>());
    case ASTNode::Type::A_LVIDENT:
        if (!reg.has_value()) {
            throw std::runtime_error("Invalid register");
        }
        return codegen_store_gblob(*reg, global_symbol_table.at(node.GetValue<size_t>()).GetName());
    case ASTNode::Type::A_IDENT:
        return codegen_load_gblob(global_symbol_table.at(node.GetValue<size_t>()).GetName());
    case ASTNode::Type::A_ASSIGN:
        return right_reg;
    case ASTNode::Type::A_VAR_DECL:
        codegen_symbol(global_symbol_table.at(node.GetValue<size_t>()).GetName());
        return 0;
    case ASTNode::Type::A_PRINT:
        codegen_printint(left_reg);
        return 0;
    default:
        throw std::runtime_error("Invalid ASTNode type");
    }
}
}  // namespace my_cpp