# Part 8: If Statements

이제 값을 비교 할 수 있게 되었으므로 `if` 문을 우리 언어에 추가 해 볼 때가 되었다.
먼저, `if`문이 어떻게 구성되는지 보고 이를 어떻게 어셈블리 언어로 바꿀 수 있는지 알아보자.

## The `IF` Syntax

`if` 문은 다음과 같은 구조를 가진다.

```
  if (condition is true)
    perform this first block of code
  else
    perform this other block of code
```

이를 어떻게 어셈블리어로 바꿀 수 있을까? 다음과 같이 `Condition`을 비교한 후에 그 결과에 따라 `jump` 를 하면 구현 가능 할 것이다.

```
       perform the opposite comparison
       jump to L1 if true
       perform the first block of code
       jump to L2
L1:
       perform the other block of code
L2:

```

여기서 `L1` 과 `L2` 는 어셈블리 언어에서 Label 을 의미한다.

## Generating The Assembly in Our Compiler

x86 어셈블리어와 관련된 부분은 여기서는 생략하고자 한다. 상세 내용은 [원글](https://github.com/DoctorWkt/acwj/tree/master/08_If_Statements) 을 참고하자.

## New Tokens and the Dangling Else

`If` 문의 구현을 위해서는 많은 새로운 토큰의 추가가 필요하다. 이와 함께 [Dangling Else 문제](https://en.wikipedia.org/wiki/Dangling_else)도 함께 피하고자 한다. 그리고 모든 문장의 묶음(Group) 은 `{` 과 `}` 로 둘러싸이도록 할 것이다. (이를 `복합문 (Compount Statement)` 이라고 부르겠다.) 또 `if` 의 조건 표현문(Condition Expression) 을 담기 위해 `(` 와 `)` 과 같은 토큰도 추가해야 하며 `if` 와 `else` 와 같은 토큰도 새로 필요하다.

이와 같은 새 토큰들은 아래와 같이 정의되었다.

```cpp
    enum Type {
        T_EOF = 0,
        T_PLUS,
        T_MINUS,
        T_STAR,
        T_SLASH,
        T_EQ,
        T_NE,
        T_LT,
        T_GT,
        T_LE,
        T_GE,
        T_INTLIT,
        T_SEMI,
        T_ASSIGN,
        T_IDENT,
        T_LBRACE,  // {
        T_RBRACE,  // }
        T_LPAREN,  // (
        T_RPAREN,  // )

        // Keywords
        T_PRINT,
        T_INT,
        T_IF,  // if
        T_ELSE,  // else
    };
```

## Scanning the Tokens

`{` 나 `(` 와 같은 한 글자 짜리 토큰을 스캔하는 것은 이제 어렵지 않게 가능하기 때문에 코드는 생략하겠다. `if` 같은 키워드들을 추가하는 일도 어렵지 않기 때문에 여기서는 내부적으로 호출하는 `keyword()` 메서드가 참고하는 테이블의 변화만 보여주곘다.

```cpp
    const std::map<std::string, Token::Type> keywords_ = {
            {"print", Token::Type::T_PRINT},
            {"int", Token::Type::T_INT},
            {"if", Token::Type::T_IF},
            {"else", Token::Type::T_ELSE},
    };
```

## The New BNF Grammar

`if` 문이 추가되면서 문법이 꽤 커지기 시작했다. 이쯤에서 한번 정리 해 보자.

```
 compound_statement: '{' '}'          // empty, i.e. no statement
      |      '{' statement '}'
      |      '{' statement statements '}'
      ;

 statement: print_statement
      |     declaration
      |     assignment_statement
      |     if_statement
      ;

 print_statement: 'print' expression ';'  ;

 declaration: 'int' identifier ';'  ;

 assignment_statement: identifier '=' expression ';'   ;

 if_statement: if_head
      |        if_head 'else' compound_statement
      ;

 if_head: 'if' '(' true_false_expression ')' compound_statement  ;

 identifier: T_IDENT ;
```

여기서 `true_false_expression` 부분은 표현되어 있지 않으나, 적당한 시점에 기술 할 것이다.

`if` 문의 문법을 한번 살펴보도록 하자. `if` 문은 `else` 없이 `if_head` 만 단독으로 있거나 `else` 와 `compound_statement` 가 함께 있는 경우로 나뉜다.

또한 각 문(`statement`) 들이 고유한 `non-terminal name` 을 가지도록 변경했고 이전의 `statement` non-terminal은 `compound_statement` 로 변경되었으며, 반드시 `{` 와 `}` 를 포함하도록 했다.

이는 `else`문 다음에 오는 `compound statement` 들은 항상 `{` 와 `}`로 둘러싸여야 한다는 것을 의미한다. 따라서 `중첩 if 문(nested if statements)` 이 있는 경우에는 다음과 같은 구조를 가지게 된다.

```
  if (condition1 is true) {
    if (condition2 is true) {
      statements;
    } else {
      statements;
    }
  } else {
    statements;
  }
```

이렇게 되면 `if` 와 `else` 를 어떻게 짝 지을지 명확해 진다. 이를 통해 `Dangling Else 문제`를 해결 할 수 있다. 이후에는 `{`과 `}`의 사용이 선택적이 되도록 바꿀 것이다.

## Parsing Compound Statements

이전의 `Parser::stmts()` 메서드는 다음과 같이 `Parser::compound_stmts()` 로 변경되었다.

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
        default:
            throw std::runtime_error("Invalid token");
        }
        if (tree != nullptr) {
            if (left == nullptr) {
                left = tree;
            } else {
                left = ASTNode::MakeAstNode(ASTNode::Type::A_GLUE, left, nullptr, tree);
            }
        }
    }
}
```

먼저, 이 메서드는 `lbrace()` 를 가장 먼저 호출하여 `{` 로 먼저 시작되도록 강제하고 있다. 그리고 이 함수는 반드시 `}` 를 만났을 때에만 빠져나올 수 있다.

> 원문에서는 `print_stmt()` 와 같은 메서드들이 AST Node들을 리턴하도록 변경되었다고 기술하고 있으나, 이 글에서는 이미 해당 내용이 적용되어있는 상태이다. 따라서 이 부분은 생략하였다.

## Parsing the IF Grammar

`Parser::if_stmt()` 메서드는 다음과 같이 구성된다. 앞서 기술한 문법과 그대로 구현되었다.

```cpp
std::shared_ptr<ASTNode> Parser::if_stmt() {
    std::shared_ptr<ASTNode> cond, true_stmt, false_stmt;

    match(Token::Type::T_IF);
    lparen();
    cond = bin_expr();

    if (!(ASTNode::Type::A_EQ <= cond->GetOp() && cond->GetOp() <= ASTNode::Type::A_GE)) {
        throw std::runtime_error("Invalid comparison operator");
    }
    rparen();

    true_stmt = compound_stmts();

    if (scanner_->Curent().GetType() == Token::Type::T_ELSE) {
        scanner_->Scan();
        false_stmt = compound_stmts();
    }

    return ASTNode::MakeAstNode(ASTNode::Type::A_IF, cond, true_stmt, false_stmt);
}
```

여기서는 `if(x-2)` 와 같이 비교 연산자를 이용해 구성한 표현식이 아닌 경우는 올바르지 않은 경우로 처리하였다.

## The Third Child

`Parser::if_stmt()` 메서드의 마지막 줄을 보면 다음과 같이 `ASTNode::MakeAstNode()` 메서드가 세 개의 자식 노드를 받는 것을 알 수 있다.

```cpp
ASTNode::MakeAstNode(ASTNode::Type::A_IF, cond, true_stmt, false_stmt);
```

이렇게 변경된 이유는 `if`문이 다음과 같은 세 개의 자식 노드를 가지기 때문이다.

- 조건문을 평가하는 자식 노드
- True 조건의 복합문
- Else 키워드에 해당하는 복합문 (없을 수도 있음)

따라서 이를 모두 표현하기 위해서 다음과 같이 ASTNode에 세 개의 자식 노드를 가지도록 변경했다.

```cpp
class ASTNode {
public:
// 생략

private:
    Type op_;
    std::shared_ptr<ASTNode> left_;
    std::shared_ptr<ASTNode> middle_;
    std::shared_ptr<ASTNode> right_;
    std::unique_ptr<Value> value_;
};
```

따라서, `A_IF` 트리는 다음과 같이 구성된다.

```
                      IF
                    / |  \
                   /  |   \
                  /   |    \
                 /    |     \
                /     |      \
               /      |       \
      condition   statements   statements
```

## Glue AST Nodes

이 장에서는 `A_GLUE` 라는 새로운 AST Node 타입도 추가되었다. 이 타입은 하나의 AST 에 여러 개의 문 (Statements) 들을 붙이기(Glue) 위해 필요하다.

앞서 구현한 `Parser::compound_stmts()` 의 끝 부분을 보자.

```cpp
            if (left == nullptr) {
                left = tree;
            } else {
                left = ASTNode::MakeAstNode(ASTNode::Type::A_GLUE, left, nullptr, tree);
            }
```

매번 반복 때 마다 새로운 서브트리가 생성되고, 새로 생성된 서브트리는 기존의 트리에 이어붙어야 한다. 예를 들어 아래와 같은 코드가 있다면 :

```
    stmt1;
    stmt2;
    stmt3;
    stmt4;
```

아래와 같이 AST가 구성되어야 한다:

```
             A_GLUE
              /  \
          A_GLUE stmt4
            /  \
        A_GLUE stmt3
          /  \
      stmt1  stmt2
```

## The Generic Code Generator

이제 새로 만든 AST 노드들은 여러 개의 자식 노드들을 가질 수 있게 되었기 때문에 코드를 생성하는 부분도 좀 더 복잡해졌다.

또한 비교 연산자와 관련된 코드를 생성 할 때에도 if 문 안에서 실행되는 비교 연산인지 (분기로의 점프를 하기 위해), 아니면 단순 비교 연산인지 (상태 레지스터를 업데이트 하기 위해) 구분 해 줄 필요가 생겼다.

이 때문에 `CodeGenerator::codegen_ast()` 메서드가 다음과 같이 부모 AST 노드가 어떤 것인지를 전달받기 위해 조금 바뀌었다.

```cpp
size_t CodeGenerator::codegen_ast(
        const ASTNode &node,
        std::optional<size_t> reg,
        const ASTNode::Type parent_op)
```

### Dealing with Specific AST Nodes

`CodeGenerator::codegen_ast()` 메서드는 새로 추가된 AST 노드 타입을 다음과 같이 다루고 있다.

```cpp
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
```

여기서 리턴되지 않고 다음 코드로 넘어간다면, _비교 연산을 제외한_ 일반적인 이항 연산자에 대한 코드 생성을 수행한다.

```cpp
    switch (node.GetOp()) {
    case ASTNode::Type::A_ADD:
        return codegen_add(left_reg, right_reg);
    case ASTNode::Type::A_SUBTRACT:
        return codegen_sub(left_reg, right_reg);
    case ASTNode::Type::A_MULTIPLY:
        return codegen_mul(left_reg, right_reg);
    case ASTNode::Type::A_DIVIDE:
        return codegen_div(left_reg, right_reg);
```

비교 연산에 대해서는 다음과 같이 수행된다.

```cpp
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
```

`codegen_compare_and_jump()` 메서드와 `codegen_compare_and_set()` 메서드에 대해서는 아래에서 다루도록 하겠다.

### Generating the IF Assembly Code

A_IF 노드 에 해당하는 코드를 생성하기 위해 아래와 같은 `CodeGenerator::codegen_if()` 메서드가 호출된다.

```cpp
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
```

실질적으로 위 코드는 다음과 같다.

```cpp
  codegen_ast(n->left, Lfalse, n->op);        // Condition and jump to Lfalse
  codegen_ast(n->mid, NOREG, n->op);          // Statements after 'if'
  codegen_jump(Lend);                         // Jump to Lend
  codegen_label(Lfalse);                      // Lfalse: label
  codegen_ast(n->right, NOREG, n->op);        // Statements after 'else'
  codegen_label(Lend);                        // Lend: label
```

## The x86-64 Code Generation Functions

이제 몇 가지 새로운 x86-64 용 코드 생성 함수를 만들어 보자. 이 과정에서 지난번에 6가지 비교 연산자를 위해 구현한 `codegen_XX()` 계열 메서드들도 새롭게 변경 될 것이다.

일반적인 비교 연산에 대한 코드를 생성 할 때에는 이제 AST의 연산자를 인자로 전달하여 이와 관련된 `set` 명령을 호출 하도록 한다.

```cpp
size_t CodeGeneratorX86::codegen_compare(
        size_t left_reg,
        size_t right_reg,
        const std::string &cmp) {
    os_ << "\tcmpq\t" << registers_names_[right_reg] << ", " << registers_names_[left_reg]
        << std::endl;
    os_ << "\t" << cmp << "\t" << bregisters_names_[right_reg] << std::endl;
    os_ << "\tandq\t$255, " << registers_names_[right_reg] << std::endl;
    register_free(left_reg);
    return right_reg;
}
```

그리고 if문에서의 비교 연산을 위해서는 `CodeGeneratorX86::codegen_compare_and_jump()` 메서드가 사용된다.

```cpp
size_t CodeGeneratorX86::codegen_compare_and_jump(
        ASTNode::Type op,
        size_t left_reg,
        size_t right_reg,
        size_t jump_reg) {
    constexpr std::array<const char *, 6> compare_cmds_ = {"jne", "je", "jge", "jle", "jg", "jl"};
    if (!(ASTNode::Type::A_EQ <= op && op <= ASTNode::Type::A_GE)) {
        throw std::runtime_error("Invalid operation");
    }
    os_ << "\tcmpq\t" << registers_names_[right_reg] << ", " << registers_names_[left_reg]
        << std::endl;
    os_ << "\t" << compare_cmds_[static_cast<size_t>(op) - static_cast<size_t>(ASTNode::Type::A_EQ)]
        << "\t"
        << ".L" << jump_reg << std::endl;
    registers_free_all();
    return kNoRegister;
}
```

## Testing the IF Statements

이제 아래와 같이 `samples/input05' 파일을 테스트 해 보도록 하자.

```c
{
  int i; int j;
  i=6; j=12;
  if (i < j) {
    print i;
  } else {
    print j;
  }
}
```

다음과 같은 결과를 얻을 수 있다.

```bash
$ build/my_cpp samples/input05
$ gcc -o out out.s
$ ./out
6 # i 는 j보다 작기 때문에 6을 출력한다.
```

## Conclusion and What's Next

여기까지 해서 우리의 언어에 `if` 라는 제어문을 추가해 보았다. 이 과정에서 몇 가지 기존 함수들을 새로 작성했고, 아마 이 구조들은 앞으로 좀 더 수정될 것이다.

이번 장에서 까다로웠던 부분은 일반적인 비교 연산과 if 문에서의 비교 연산을 구분하여 각기 다른 코드를 생성하도록 하는 부분이었다. 이 장에서 구현한 방법은 비교 연산의 부모 노드가 어떤 것인지를 함께 입력받아 그에 따라 다른 코드를 생성하도록 하는 방법이었다.

다음 장에서는 `WHILE` 문 이라는 또 다른 제어문을 추가 해 볼 것이다.
