# Part 9: While Loops

이번 장에서는 WHILE loop를 우리 언어에 추가 해 볼 것이다. 어떤 면에서는 WHILE loop 는 항상 반복문의 처음으로 돌아온다는 것을 제외하면 else 절이 없는 IF 문과 매우 비슷하다고 볼 수 있다.

즉, 아래 코드는:

```
  while (condition is true) {
    statements;
  }
```

다음 처럼 컴파일 될 수 있다:

```
Lstart: evaluate condition
	jump to Lend if condition false
	statements
	jump to Lstart
Lend:
```

즉, IF 문에서 사용했던 스캐닝, 파싱, 코드 생성 기법들을 빌려와 약간 수정만 함으로써 WHILE 문을 구현 할 수 있다.

어떻게 만들 수 있는지 살펴 보자.

## 새 토큰

우선, _while_ 키워드를 지원하기 위해 _T_WHILE_ 이라는 새 키워드를 추가해야 한다. defs.hpp 아 scan.cpp 에 다음과 같이 추가하였다.

``` cpp
// defs.hpp

class Token {
 public:
  // Types of tokens
  enum Type {
    T_EOF = 0,

    // ...

    // Keywords
    T_PRINT,
    T_INT,
    T_IF,
    T_ELSE,
    T_WHILE, // 새로 추가된 부분
  };
```

``` cpp
// scan.cpp
const std::map<std::string, Token::Type> keywords_ = {
    {"print", Token::Type::T_PRINT}, {"int", Token::Type::T_INT},
    {"if", Token::Type::T_IF},       {"else", Token::Type::T_ELSE},
    {"while", Token::Type::T_WHILE}, // 새로 추가된 부분
};
```

그리고 while 키워드를 인식하여 파싱할 수 있도록 compound_stmts() 를 아래와 같이 수정하였다.
```cpp
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
        // 새로 추가한 부분
        case Token::Type::T_WHILE:
            tree = while_stmt();
            break;
```

## While 문 파싱하기

WHILE 반복문의 BNF 문법은 아래와 같다:
```
// while_statement: 'while' '(' true_false_expression ')' compound_statement  ;
```

parser.cpp 에 이를 파싱하기 위해 메서드를 하나 추가하자. IF문과 상당히 비슷하다.

```cpp
std::shared_ptr<ASTNode> Parser::while_stmt() {
    std::shared_ptr<ASTNode> cond, body;

    match(Token::Type::T_WHILE);
    lparen();
    cond = bin_expr();

    if (!(ASTNode::Type::A_EQ <= cond->GetOp() && cond->GetOp() <= ASTNode::Type::A_GE)) {
        throw std::runtime_error("Invalid comparison operator");
    }
    rparen();

    body = compound_stmts();

    return ASTNode::MakeAstNode(ASTNode::Type::A_WHILE, cond, nullptr, body);
}
```

먼저, 새로운 AST 를 추가하기 위해 _A_WHILE_ 이라는 새 AST Node 타입을 추가했다. 이 노드는 조건문(condition) 에 대한 sub-tree를 왼쪽 child node에, 본문(body)에 대한 sub-tree를 오른쪽 child node에 담고 있다. 

## Generic Code 생성

WHILE loop에 대한 코드를 생성하기 위해서는 시작과 끝에 대한 label, 조건문 검사 및 결과에 따른 Jump 를 위한 코드 추가가 필요하다. gen.cpp 에서 아래와 같이 구현 할 수 있다. (if문과 비슷하지만 훨씬 간단하다.)

```cpp
size_t CodeGenerator::codegen_while(const ASTNode &while_stmt) {
    size_t label_start, label_end;
    label_start = label_new();
    label_end = label_new();

    codegen_label(label_start);
    codegen_ast(*(while_stmt.GetLeft()), label_end, while_stmt.GetOp());
    registers_free_all();

    codegen_ast(*(while_stmt.GetRight()), std::nullopt, while_stmt.GetOp());
    registers_free_all();

    codegen_jump(label_start);
    codegen_label(label_end);
    return kNoRegister;
}
```

또한 비교 연산 시 부모 노드가 if 문 뿐 아니라 while 문이 될 수 있기 때문에 아래와 같이 codegen_ast() 메서드의 수정이 필요하다.


마지막으로 while AST Node를 만났을 때 코드를 생성하도록 codegen_ast() 메서드를 수정해야 한다.
```cpp
size_t CodeGenerator::codegen_ast(
        const ASTNode &node,
        std::optional<size_t> reg,
        const ASTNode::Type parent_op) {
    size_t left_reg, right_reg;

    switch (node.GetOp()) {
    case ASTNode::Type::A_IF:
        return codegen_if(node);
        
    // 새로 추가된 부분
    case ASTNode::Type::A_WHILE:
        return codegen_while(node);

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
// 중간 생략
    case ASTNode::Type::A_GE: {
        // 새로 추가된 부분
        if (parent_op == ASTNode::Type::A_IF || parent_op == ASTNode::Type::A_WHILE) {
            return codegen_compare_and_jump(node.GetOp(), left_reg, right_reg, *reg);
        } else {
            return codegen_compare_and_set(node.GetOp(), left_reg, right_reg);
        }
    }
```

### 결론

이 장에서는 IF 문을 만들면서 이미 구현 해 둔 것들을 이용하여 WHILE 문을 쉽게 만들어 보았다. 다음 장에서는 FOR 반복문을 추가 해 보자.