#include "gen.hpp"

#include "glob_vars.hpp"
// Standard includes
// C++ Standard
// C Standard

namespace my_cpp {
CodeGenerator::CodeGenerator(std::ostream &os) : os_(os) {}
void CodeGenerator::GenerateCode(
    const std::vector<std::shared_ptr<ASTNode>> &stmts) {
  codegen_preemble();
  for (const auto &stmt : stmts) {
    codegen_ast(*stmt);
    registers_free_all();
  }

  codegen_postemble();
}

size_t CodeGenerator::codegen_ast(const ASTNode &node,
                                  std::optional<size_t> reg) {
  size_t left_reg, right_reg;

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
    case ASTNode::Type::A_INTLIT:
      return codegen_load_int(node.GetValue<int>());
    case ASTNode::Type::A_LVIDENT:
      if (!reg.has_value()) {
        throw std::runtime_error("Invalid register");
      }
      return codegen_store_gblob(
          *reg, global_symbol_table.at(node.GetValue<size_t>()).GetName());
    case ASTNode::Type::A_IDENT:
      return codegen_load_gblob(
          global_symbol_table.at(node.GetValue<size_t>()).GetName());
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