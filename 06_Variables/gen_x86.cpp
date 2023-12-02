#include "gen_x86.hpp"
// Standard includes
// C++ Standard
// C Standard

namespace my_cpp {
CodeGeneratorX86::CodeGeneratorX86(std::ostream &os)
    : CodeGenerator(os),
      registers_(4, false),
      registers_names_{"%r8", "%r9", "%r10", "%r11"} {}

size_t CodeGeneratorX86::registers_alloc() {
  for (size_t i = 0; i < registers_.size(); ++i) {
    if (!registers_[i]) {
      registers_[i] = true;
      return i;
    }
  }
  throw std::runtime_error("No free registers");
}

void CodeGeneratorX86::registers_free(size_t reg) {
  if (registers_[reg] != true) {
    throw std::runtime_error("Register is not allocated");
  }
  registers_[reg] = false;
}

void CodeGeneratorX86::registers_free_all() {
  for (size_t i = 0; i < registers_.size(); ++i) {
    registers_[i] = false;
  }
}

void CodeGeneratorX86::codegen_preemble() {
  registers_free_all();
  os_ << "\t.text" << std::endl
      << ".LC0:" << std::endl
      << "\t.string\t\"%d\\n\"" << std::endl
      << "printint:" << std::endl
      << "\tpushq\t%rbp" << std::endl
      << "\tmovq\t%rsp, %rbp" << std::endl
      << "\tsubq\t$16, %rsp" << std::endl
      << "\tmovl\t%edi, -4(%rbp)" << std::endl
      << "\tmovl\t-4(%rbp), %eax" << std::endl
      << "\tmovl\t%eax, %esi" << std::endl
      << "\tleaq	.LC0(%rip), %rdi" << std::endl
      << "\tmovl	$0, %eax" << std::endl
      << "\tcall	printf@PLT" << std::endl
      << "\tnop" << std::endl
      << "\tleave" << std::endl
      << "\tret" << std::endl
      << "" << std::endl
      << "\t.globl\tmain" << std::endl
      << "\t.type\tmain, @function" << std::endl
      << "main:" << std::endl
      << "\tpushq\t%rbp" << std::endl
      << "\tmovq	%rsp, %rbp" << std::endl;
}

void CodeGeneratorX86::codegen_postemble() {
  os_ << "\tmovl	$0, %eax" << std::endl
      << "\tpopq	%rbp" << std::endl
      << "\tret" << std::endl;
}

size_t CodeGeneratorX86::codegen_add(size_t left_reg, size_t right_reg) {
  os_ << "\taddq\t" << registers_names_[left_reg] << ", "
      << registers_names_[right_reg] << std::endl;
  registers_free(left_reg);
  return right_reg;
}

size_t CodeGeneratorX86::codegen_sub(size_t left_reg, size_t right_reg) {
  os_ << "\tsubq\t" << registers_names_[right_reg] << ", "
      << registers_names_[left_reg] << std::endl;
  registers_free(right_reg);
  return left_reg;
}

size_t CodeGeneratorX86::codegen_mul(size_t left_reg, size_t right_reg) {
  os_ << "\timulq\t" << registers_names_[left_reg] << ", "
      << registers_names_[right_reg] << std::endl;
  registers_free(left_reg);
  return right_reg;
}

size_t CodeGeneratorX86::codegen_div(size_t left_reg, size_t right_reg) {
  os_ << "\tmovq\t" << registers_names_[left_reg] << ", %rax" << std::endl;
  os_ << "\tcqo" << std::endl;
  os_ << "\tidivq\t" << registers_names_[right_reg] << std::endl;
  os_ << "\tmovq\t%rax, " << registers_names_[right_reg] << std::endl;
  registers_free(left_reg);
  return right_reg;
}

size_t CodeGeneratorX86::codegen_load_int(size_t value) {
  size_t reg = registers_alloc();
  os_ << "\tmovq\t$" << value << ", " << registers_names_[reg] << std::endl;
  return reg;
}

size_t CodeGeneratorX86::codegen_load_gblob(const std::string &identifier) {
  size_t reg = registers_alloc();
  os_ << "\tmovq\t" << identifier << "(%rip), " << registers_names_[reg]
      << std::endl;
  return reg;
}

size_t CodeGeneratorX86::codegen_store_gblob(size_t reg,
                                             const std::string &identifier) {
  os_ << "\tmovq\t" << registers_names_[reg] << ", " << identifier << "(%rip)"
      << std::endl;
  registers_free(reg);
  return reg;
}

void CodeGeneratorX86::codegen_symbol(const std::string &identifier) {
  os_ << "\t.comm\t" << identifier << ",8,8" << std::endl;
}

void CodeGeneratorX86::codegen_printint(size_t reg) {
  os_ << "\tmovq\t" << registers_names_[reg] << ", %rdi" << std::endl;
  os_ << "\tcall\tprintint" << std::endl;
  registers_free(reg);
}
}  // namespace my_cpp