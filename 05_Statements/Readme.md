# Part 5: Statements

이 장에서는 우리의 언어에 다음과 같은 문(Statement) 를 지원하도록 기능을 추가 해 보자.

```cpp
   print 2 + 3 * 5;
   print 18 - 6/3 + 4*2;
```

물론, 우리는 스캐닝 과정에서 공백을 무시하기 때문에 한 줄에 하나의 문장만 있을 필요는 없다.
대신 여기서 각 문장은 `print` 라는 키워드로 시작하고, 세미콜론(`;`) 으로 종료되도록 정의 해 보자. 때문에 이 두 개의 토큰이 새로 추가되어야 한다.

## BNF Description of the Grammar

이미 표현식(Expression)에 대한 BNF 표현을 정의 했듯이, 위 예제와 같은 타입의 문 들에 대한 BNF 표현을 정의해 보자.

```
statements: statement
     | statement statements
     ;

statement: 'print' expression ';'
     ;
```

입력 파일은 여러 개의 문 으로 이루어진다. 단 하나의 문 일 수도 있고, 또는 연속된 여러 문이 있을 수도 있다. 각 문은 `print` 키워드로 시작하고 표현식(Expression) 이 중간에 있으며, 세미;콜론(`;`) 으로 끝난다.

## Changes to the Lexical Scanner

위 문법들을 구현하기 위해서는 몇 가지 코드들을 추가해야 한다. 먼저, 스캐너부터 시작해 보자.

세미콜론을 위한 토큰을 추가하는 작업은 매우 간단하기 때문에 여기서는 `print` 키워드만 살펴보자. 앞으로 매우 다양한 키워드들도 추가해야 하고 또한 변수들의 이름(식별자, Identifier) 들도 지원해야 하기 때문에 이런 기능들의 지원도 함께 고려되어야 한다.

우선, 키워드와 변수명 등의 식별자들은 글자(Character)들의 연속이기 때문에 이를 인식하기 위한 메서드가 필요하다. `Scanner::scan_id` 메서드가 이를 담당한다.

```cpp
std::string Scanner::scan_id(char c) {
    std::string s;
    while (std::isalpha(c) || isdigit(c) || '_' == c) {
        s += c;
        c = next_token();
    }
    set_put_back(c);
    return s;
}
```

또한 이로부터 키워드들을 구분 해 낼 필요가 있다. `Scanner::keyword` 메서드가 이를 담당한다.

```cpp
class Scanner {
// 생략
private:
/// 생략
    const std::map<std::string, Token::Type> keywords_ = {
            {"print", Token::Type::T_PRINT},
    };
};

Token::Type Scanner::keyword(const std::string &s) const {
    auto it = keywords_.find(s);
    if (it != keywords_.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown keyword : " + s);
}
```

> 이 문서에서는 C++로 만들고 있어 STL의 다양한 자료구조를 통해 비교적 간단하게 위 메서드들을 구현하고 있다. 원글에서는 C로 컴파일러를 작성하고 있기 때문에 이를 직접 구현하여 코드가 다소 복잡하다.

이제 `Scanner::Scan` 메서드의 끝에 세미콜론과 키워드들을 인식하는 코드를 추가 해 보자.

```cpp
void Scanner::Scan() {
    char c = skip();
    switch (c) {
    case EOF:
        current_ = std::make_unique<Token>(Token::Type::T_EOF);
        break;
    // 생략
    default:
        if (is_digit(c)) {
            int n = scan_int(c);
            current_ = std::make_unique<Token>(
                    Token::Type::T_INTLIT, std::make_unique<Value>(Value{n}));
        } else if (std::isalpha(c) || '_' == c) {
            std::string s = scan_id(c);
            Token::Type type = keyword(s);
            current_ = std::make_unique<Token>(type);
        } else {
            std::stringstream ss;
            ss << "Invalid character : " << c;
            throw std::runtime_error(ss.str());
        }
        break;
    }
}
```

## Changes to the Expression Parser

이제까지 우리가 만든 컴파일러는 오직 하나의 표현식을 가진 입력 파일만을 받아들였다. 새로 정의한 문법에서는 각 표현식들은 세미콜론으로 끝난다. 따라서 `Parser::bin_expr` 메서드는 세미콜론을 만나면 결과값을 리턴하도록 수정되어야 한다.

```cpp
std::shared_ptr<ASTNode> Parser::bin_expr(const size_t prev_precedence) {
    std::shared_ptr<ASTNode> left, right;
    left = primary();

    // 세미콜론을 만나면 left 표현식을 리턴한다.
    if (scanner_->Curent().GetType() == Token::Type::T_SEMI) {
        return left;
    }
    auto current_op_type = scanner_->Curent().GetType();
    auto current_op_precedence = get_priority(scanner_->Curent());
    while (current_op_precedence > prev_precedence) {
        scanner_->Scan();
        right = bin_expr(current_op_precedence);
        left = ASTNode::MakeAstNode(arith_op(current_op_type), left, right);
        auto token = scanner_->Curent().GetType();
        current_op_type = scanner_->Curent().GetType();

        // 세미콜론을 만나면 루프를 멈추고 left 표현식을 리턴한다.
        if (current_op_type == Token::Type::T_SEMI) {
            break;
        }
        current_op_precedence = get_priority(scanner_->Curent());
    }
    return left;
}
```

## Changes to the Code Generator

> 원문과 다르게, 이 부분은 이미 `CodeGeneratorX86` 클래스가 `CodeGenerator` 를 상속받음으로서 자연스럽게 구현되었다. 따라서 여기서는 따로 기술하지 않겠다.

## Adding the Parser for Statements

> 원문에서는 이 부분을 stmt.c 라는 새 파일을 생성하고, 각 문을 인식 할 때 마다 바로 코드를 생성한다. 틀린 방법은 아니지만, 코드 생성 부분과 파싱 부분이 함께 섞지 않고 분리하는게 낫다고 판단하여 여기서는 원문과 다르게 구현하였다.

이제, 우리가 정의한 문(Statement) 에 대한 문법을 파싱하는 코드를 추가 해 보자.

```cpp
std::vector<std::shared_ptr<ASTNode>> Parser::stmts() {
    std::vector<std::shared_ptr<ASTNode>> result;
    while (true) {
        match(Token::Type::T_PRINT);
        auto tree = bin_expr();
        semi();
        result.push_back(tree);

        if (scanner_->Curent().GetType() == Token::Type::T_EOF) {
            return result;
        }
    }
}

std::vector<std::shared_ptr<ASTNode>> Parser::Parse() {
    return stmts();
}
```

`Parser::stmts` 메서드는 먼저, 문법대로 `print` 토큰이 나타나는지 확인 후, 그 다음 이어지는 표현식들에 대한 AST를 생성한다. 그리고 마지막으로 세미콜론 토큰을 인식한다.
이렇게 루프를 반복하며 인식된 문장들은 벡터에 저장되었다가 한꺼번에 리턴된다.

이 때문에 `Parser::Parse` 메서드는 표현식 하나의 AST 를 리턴하는 대신 앞서 만든 문장들의 AST 벡터를 리턴하도록 변경되었다. 따라서 `CodeGenerator::GenerateCode` 메서드도 다음과 같이 변경되어야 한다.

```cpp
void CodeGenerator::GenerateCode(
    const std::vector<std::shared_ptr<ASTNode>> &stmts) {
  codegen_preamble();
  for (const auto &stmt : stmts) {
    size_t reg = codegen_ast(*stmt);
    codegen_printint(reg);
    registers_free_all();
  }

  codegen_postamble();
}
```

입력받은 벡터 안에 있는 문 들을 순차적으로 읽어와 이에 대한 코드를 생성하도록 변경되었다.

## Some Helper Functions

위 코드들을 보면 몇몇 유틸리티 메서드들이 추가되었다. 아래와 같은 코드로 설명은 대신하겠다.

```cpp
void Parser::match(Token::Type type) {
    if (scanner_->Curent().GetType() == type) {
        scanner_->Scan();
    } else {
        std::stringstream ss;
        ss << type << " expected at line " << scanner_->GetLine();
    }
}

void Parser::semi() {
    match(Token::Type::T_SEMI);
}
```

## Changes to main()

> 원글에는 `statements` 함수가 추가되어 `main` 함수도 이를 호출하도록 변경되었지만 여기서는 `Parser::Parse()` 함수의 인터페이스가 변경되는 정도로 수정되어서 main 함수는 변경되지 않는다.

## Trying It Out

이제 테스트를 한번 해 보자. 아래와 같이 테스트를 해 보자. 입력 파일의 토큰이 어떠한 모양으로 있어도 문법만 제대로 맞다면 동작하는 것을 확인 할 수 있다.

```bash
$ cat samples/input01
print 12 * 3;
print
   18 - 2
      * 4; print
1 + 2 +
  9 - 5/2 + 3*5;
$ build/my_cpp samples/input01
$ cat out.s
        .text
.LC0:
        .string "%d\n"
printint:
        pushq   %rbp
        movq    %rsp, %rbp
        subq    $16, %rsp
        movl    %edi, -4(%rbp)
        movl    -4(%rbp), %eax
        movl    %eax, %esi
        leaq    .LC0(%rip), %rdi
        movl    $0, %eax
        call    printf@PLT
        nop
        leave
        ret

        .globl  main
        .type   main, @function
main:
        pushq   %rbp
        movq    %rsp, %rbp
        movq    $12, %r8
        movq    $3, %r9
        imulq   %r8, %r9
        movq    %r9, %rdi
        call    printint
        movq    $18, %r8
        movq    $2, %r9
        movq    $4, %r10
        imulq   %r9, %r10
        subq    %r10, %r8
        movq    %r8, %rdi
        call    printint
        movq    $1, %r8
        movq    $2, %r9
        addq    %r8, %r9
        movq    $9, %r8
        addq    %r9, %r8
        movq    $5, %r9
        movq    $2, %r10
        movq    %r9, %rax
        cqo
        idivq   %r10
        movq    %rax, %r10
        subq    %r10, %r8
        movq    $3, %r9
        movq    $5, %r10
        imulq   %r9, %r10
        addq    %r8, %r10
        movq    %r10, %rdi
        call    printint
        movl    $0, %eax
        popq    %rbp
        ret
$ gcc -o out out.s
$ ./out
36
10
25
```

## Conclusion and What's Next

이번 장에서는 우리의 언어에 `문(Statement)` 을 추가 해 보았다. BNF 방식으로 재귀적으로 표현했지만, 일단은 반복문으로 구현했다. 추후 재귀 방식으로 구현을 바꿀 것이다.

_문_ 의 구현을 위해 키워드와 식별자를 구분하는 기능을 스캐너에 추가했고 이에 맞추어 파서도 수정하였다.

다음 장에서는 언어에 변수를 도입 해 볼 것이다. 조금 큰 작업이 될 것이니 차근차근 해 보자.
