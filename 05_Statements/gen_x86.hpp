#pragma once

#include "gen.hpp"
// Standard includes
// C++ Standard
#include <string>
#include <vector>
// C Standard

namespace my_cpp {
class CodeGeneratorX86 : public CodeGenerator {
 public:
  CodeGeneratorX86(std::ostream &os);
  virtual ~CodeGeneratorX86() = default;

 private:
  std::vector<bool> registers_;
  std::vector<std::string> registers_names_;
  size_t registers_alloc() override final;
  void registers_free(size_t reg) override final;
  void registers_free_all() override final;

  void codegen_preamble() override final;
  void codegen_postamble() override final;

  size_t codegen_add(size_t left_reg, size_t right_reg) override final;
  size_t codegen_sub(size_t left_reg, size_t right_reg) override final;
  size_t codegen_mul(size_t left_reg, size_t right_reg) override final;
  size_t codegen_div(size_t left_reg, size_t right_reg) override final;
  size_t codegen_load(size_t value) override final;

  void codegen_printint(size_t reg) override final;
};
}  // namespace my_cpp