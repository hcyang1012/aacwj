#pragma once

#include "defs.hpp"
// Standard includes
// C++ Standard
#include <memory>
#include <ostream>
// C Standard
#include <cstddef>

namespace my_cpp {
class ASTNode {
public:
    enum class Type {
        A_ADD,
        A_SUBTRACT,
        A_MULTIPLY,
        A_DIVIDE,
        A_INTLIT,
    };

    ASTNode(Type op,
            std::shared_ptr<ASTNode> left,
            std::shared_ptr<ASTNode> right,
            std::unique_ptr<Value> value = nullptr);
    Type GetType() const;
    template <typename T>
    T GetValue() const;
    std::shared_ptr<ASTNode> GetLeft() const;
    std::shared_ptr<ASTNode> GetRight() const;

    static std::shared_ptr<ASTNode> MakeAstNode(
            Type op,
            std::shared_ptr<ASTNode> left,
            std::shared_ptr<ASTNode> right,
            std::unique_ptr<Value> value = nullptr);
    static std::shared_ptr<ASTNode> MakeAstLeaf(Type op, std::unique_ptr<Value> value = nullptr);
    static std::shared_ptr<ASTNode> MakeAstUnary(
            Type op,
            std::shared_ptr<ASTNode> left,
            std::unique_ptr<Value> value = nullptr);

private:
    Type op_;
    std::shared_ptr<ASTNode> left_;
    std::shared_ptr<ASTNode> right_;
    std::unique_ptr<Value> value_;
};

namespace utility {
int Evaluate(const ASTNode &node);
}
}
std::ostream &operator<<(std::ostream &os, const my_cpp::ASTNode::Type &type);