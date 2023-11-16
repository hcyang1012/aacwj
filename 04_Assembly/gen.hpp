#pragma once

#include "ast.hpp"
// Standard includes
// C++ Standard
#include <ostream>
// C Standard

namespace my_cpp {
class CodeGenerator {
public:
    CodeGenerator(std::ostream &os);
    virtual ~CodeGenerator() = default;
    void GenerateCode(const ASTNode &node);

protected:
    virtual void codegen_preemble() {
    }
    virtual void codegen_printint(size_t reg) {
        throw std::runtime_error("Not implemented");
    };
    virtual void codegen_postemble() {
    }

    virtual size_t codegen_add(size_t left_reg, size_t right_reg) {
        throw std::runtime_error("Not implemented");
    };
    virtual size_t codegen_sub(size_t left_reg, size_t right_reg) {
        throw std::runtime_error("Not implemented");
    };
    virtual size_t codegen_mul(size_t left_reg, size_t right_reg) {
        throw std::runtime_error("Not implemented");
    };
    virtual size_t codegen_div(size_t left_reg, size_t right_reg) {
        throw std::runtime_error("Not implemented");
    };
    virtual size_t codegen_load(size_t value) {
        throw std::runtime_error("Not implemented");
    };

    virtual void registers_free_all() {
        throw std::runtime_error("Not implemented");
    };
    virtual size_t registers_alloc() {
        throw std::runtime_error("Not implemented");
    };
    virtual void registers_free(size_t reg) {
        throw std::runtime_error("Not implemented");
    };

    std::ostream &os_;

private:
    size_t codegen_ast(const ASTNode &node);
};
}
std::ostream &operator<<(std::ostream &os, const my_cpp::ASTNode::Type &type);