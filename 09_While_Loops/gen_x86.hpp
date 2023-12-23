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
    std::vector<std::string> bregisters_names_;
    size_t registers_alloc() override final;
    void register_free(size_t reg) override final;
    void registers_free_all() override final;

    void codegen_preemble() override final;
    void codegen_postemble() override final;

    size_t codegen_add(size_t left_reg, size_t right_reg) override final;
    size_t codegen_sub(size_t left_reg, size_t right_reg) override final;
    size_t codegen_mul(size_t left_reg, size_t right_reg) override final;
    size_t codegen_div(size_t left_reg, size_t right_reg) override final;
    size_t codegen_load_int(size_t value) override final;
    size_t codegen_load_gblob(const std::string &identifier) override final;
    size_t codegen_store_gblob(size_t reg, const std::string &identifier) override final;
    void codegen_label(size_t label) override final;
    void codegen_jump(size_t label) override final;

    size_t codegen_compare_and_jump(
            ASTNode::Type op,
            size_t left_reg,
            size_t right_reg,
            size_t jump_reg) override final;
    size_t codegen_compare_and_set(ASTNode::Type op, size_t left_reg, size_t right_reg);

    size_t codegen_compare(size_t left_reg, size_t right_reg, const std::string &cmp);

    void codegen_symbol(const std::string &identifier) override final;

    void codegen_printint(size_t reg) override final;
};
}  // namespace my_cpp