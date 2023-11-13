#include "ast.hpp"

// Standard includes
// C++ Standard
#include <iostream>
#include <stdexcept>
// C Standard

namespace my_cpp {

ASTNode::ASTNode(
        Type op,
        std::shared_ptr<ASTNode> left,
        std::shared_ptr<ASTNode> right,
        std::unique_ptr<Value> value)
    : op_(op), left_(left), right_(right), value_(std::move(value)) {
}

ASTNode::Type ASTNode::GetType() const {
    return op_;
}

template <typename T>
T ASTNode::GetValue() const {
    if (value_ == nullptr) {
        throw std::runtime_error("ASTNode has no value");
    }
    switch (op_) {
    case Type::A_INTLIT:
        return static_cast<T>(value_->int_value_);
    default:
        throw std::runtime_error("Invalid ASTNode type");
    }
}

std::shared_ptr<ASTNode> ASTNode::GetLeft() const {
    return left_;
}

std::shared_ptr<ASTNode> ASTNode::GetRight() const {
    return right_;
}

std::shared_ptr<ASTNode> ASTNode::MakeAstNode(
        Type op,
        std::shared_ptr<ASTNode> left,
        std::shared_ptr<ASTNode> right,
        std::unique_ptr<Value> value) {
    return std::make_shared<ASTNode>(op, left, right, std::move(value));
}

std::shared_ptr<ASTNode> ASTNode::MakeAstLeaf(Type op, std::unique_ptr<Value> value) {
    return std::make_shared<ASTNode>(op, nullptr, nullptr, std::move(value));
}

std::shared_ptr<ASTNode> ASTNode::MakeAstUnary(
        Type op,
        std::shared_ptr<ASTNode> left,
        std::unique_ptr<Value> value) {
    return std::make_shared<ASTNode>(op, left, nullptr, std::move(value));
}

namespace utility {
int Evaluate(const ASTNode &node) {
    auto left = node.GetLeft();
    auto right = node.GetRight();

    auto left_value = 0;
    if (left != nullptr) {
        left_value = Evaluate(*left);
    }

    auto right_value = 0;
    if (right != nullptr) {
        right_value = Evaluate(*right);
    }

    if (node.GetType() == ASTNode::Type::A_INTLIT) {
        std::cout << "A_INTLIT: " << node.GetValue<int>() << std::endl;
    } else {
        std::cout << left_value << " " << node.GetType() << " " << right_value << std::endl;
    }

    switch (node.GetType()) {
    case ASTNode::Type::A_ADD:
        return left_value + right_value;
    case ASTNode::Type::A_SUBTRACT:
        return left_value - right_value;
    case ASTNode::Type::A_MULTIPLY:
        return left_value * right_value;
    case ASTNode::Type::A_DIVIDE:
        return left_value / right_value;
    case ASTNode::Type::A_INTLIT:
        return node.GetValue<int>();
    default:
        throw std::runtime_error("Invalid ASTNode type");
    }
}
}
}

std::ostream &operator<<(std::ostream &os, const my_cpp::ASTNode::Type &type) {
    switch (type) {
    case my_cpp::ASTNode::Type::A_ADD:
        os << "A_ADD";
        break;
    case my_cpp::ASTNode::Type::A_SUBTRACT:
        os << "A_SUBTRACT";
        break;
    case my_cpp::ASTNode::Type::A_MULTIPLY:
        os << "A_MULTIPLY";
        break;
    case my_cpp::ASTNode::Type::A_DIVIDE:
        os << "A_DIVIDE";
        break;
    case my_cpp::ASTNode::Type::A_INTLIT:
        os << "A_INTLIT";
        break;
    default:
        os << "Invalid ASTNode::Type";
        break;
    }
    return os;
}
