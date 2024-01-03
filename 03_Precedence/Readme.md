# Part 3: Operator Precedence

이전 장에서 만든 파서는 언어의 의미(Semantic) 는 파악하지 못 하고 단지 문법적인 부분만 확인한다. 예를 들어, `2 * 3 + 4 * 5` 과 같은 표현식은 제대로 계산 해 내지 못 한다. 왜냐하면 아래와 같은 AST를 만들어 내기 때문이다:

```
     *
    / \
   2   +
      / \
     3   *
        / \
       4   5
```

제대로 계산해 내기 위해서는 아래와 같이 AST가 생성되어져야 한다.

```
          +
         / \
        /   \
       /     \
      *       *
     / \     / \
    2   3   4   5
```

이를 해결하기 위해서는 파서가 연산자 우선순위(Operator Precedence)를 고려하도록 파서를 변경해야 한다. 이를 위해서는 크게 두 가지 방법이 있다.

- 문법에 명시적으로 표시하여 구현하는 방법
- 현 구현에 연산자 우선순위 테이블을 반영

> [원글](https://github.com/DoctorWkt/acwj/tree/master/03_Precedence) 에는 위 두 가지 방법 모두 설명하고 있지만 여기서는 실질적으로 많이 쓰이는 두 번째 방법만 기술하겠다.

## Making the Operator Precedence Explicit

원글 참조

## The Alternative: Pratt Parsing

문법에 모든 연산자 우선순위를 고려하는 방법은 연산자가 많아질 수록 코드의 구현이 갈수록 복잡해 질 수 있다. 그 대안으로 연산자 우선순위에 대한 테이블을 만들고 이를 활용하는 [Pratt Parsing](https://en.wikipedia.org/wiki/Operator-precedence_parser#Pratt_parsing) 이라는 방법이 있다.

다음 설명으로 넘어가기 전에 Boy Nystrom의 [Pratt Parsers: Expression Parsing Made Easy](https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/) 라는 글을 한번 읽어보길 권한다. 다소 복잡한 로직을 이해하기 쉽게 설명 해 두었다.

Pratt Parsing 로직은 `Parser` Class 에 구현 해 두었다. 먼저 연산자 우선순위 테이블을 정의해야 한다. 이는 `parser.hpp` 에서 찾을 수 있다.

```cpp
class Parser {
public:
   // 생략
private:
   // 생략
   // 연산자 우선순위 테이블
    static constexpr size_t kPriority[] = {
            0,   // EOF
            10,  // +
            10,  // -
            20,  // *
            20,  // /
            0,   // INTLIT
    };
};
```

이 테이블을 기반으로 각 연산자의 우선순위를 찾는 코드는 `Parser::get_priority` 메서드가 담당한다.

```cpp
size_t Parser::get_priority(const Token &token) const {
    if (token.GetType() >= sizeof(kPriority) / sizeof(kPriority[0])) {
        throw std::runtime_error("Invalid token");
    }
    auto prec = kPriority[static_cast<size_t>(token.GetType())];
    if (prec == 0) {
        throw std::runtime_error("Invalid token at line " + std::to_string(scanner_->GetLine()));
    }
    return prec;
}
```

여기서, 더 큰 숫자는 더 높은 우선순위를 뜻한다. 이를 기반으로 `Parser::bin_expr` 메서드는 아래와 같이 변경되었다.

```cpp
std::shared_ptr<ASTNode> Parser::bin_expr(const size_t prev_precedence) {
    std::shared_ptr<ASTNode> left, right;

    // 먼저, 정수 리터럴(Literal)은 왼쪽 Subtree에 위치시킨다.
    left = primary();

    // 왼쪽에 아무런 토큰이 없었다면 그냥 리턴한다.
    if (scanner_->Curent().GetType() == Token::Type::T_EOF) {
        return left;
    }

    // 새로 들어오는 연산자의 우선순위가 이전 연산자의 우선순위보다
    // 높은 동안에는 파싱을 계속 한다.
    auto current_op_type = scanner_->Curent().GetType();
    auto current_op_precedence = get_priority(scanner_->Curent());
    while (current_op_precedence > prev_precedence) {
        // 다음 토큰을 읽는다
        scanner_->Scan();

        // 재귀적으로 (Recursively) 다음 표현식들을 파싱한다.
        // 단, 현재 연산자 우선순위를 함께 전달하여
        // 더 높은 연산자를 만나는 동안만 수행한다.
        right = bin_expr(current_op_precedence);

        // Left, Right를 결합하여 새로운 AST를 생성한다.
        left = ASTNode::MakeAstNode(arith_op(current_op_type), left, right);

        // 끝에 도달했으면 종료한다.
        auto token = scanner_->Curent().GetType();
        if (token == Token::Type::T_EOF) {
            break;
        }

        // 다음 연산자를 읽어 파싱을 계속 한다.
        current_op_type = scanner_->Curent().GetType();
        current_op_precedence = get_priority(scanner_->Curent());
    }
    return left;
}
```

이 메서드는 여전히 재귀적으로 호출된다. 다만 이전과 다르게 매 호출 시 직전까지 파싱했던 연산자의 우선순위를 함께 전달한다. `main()` 함수에서 제일 처음 이 메서드를 호출 할 때에는 가장 낮은 우선순위인 `0`을 전달하여 어떤 연산자든 파싱을 하도록 한다.

Pratt Parsing 에서는 더 높은 우선순위의 연산자를 만났을 경우 `primary()` 메서드로 다음 정수 리터럴을 읽는 대신 스스로를 재귀호출함으로서 현재 파싱하고 있는 연산자의 우선순위를 높인다.

그리고 현재 연산자 우선순위와 같거나 낮은 연산자를 만나면 파싱을 종료하고 리턴한다. 이렇게 함으로써 생성되는 AST Subtree는 현재 자기보다 더 높은 연산자 우선순위를 가지는 연산들만 포함하게 된다.

## Putting Both Parsers Into Action

> 제목은 `Both Parsers`이지만 여기서는 Pratt Parser만 설명한다.

구현한 파서를 테스트 해 보자. 우선순위를 반영하여 원하는 결과가 나오는 것을 확인 할 수 있다.

```bash
$ cat samples/input01
2 + 3 * 5 - 8 / 3
$ build/my_cpp samples/input01
A_INTLIT: 2
A_INTLIT: 3
A_INTLIT: 5
3 A_MULTIPLY 5
2 A_ADD 15
A_INTLIT: 8
A_INTLIT: 3
8 A_DIVIDE 3
17 A_SUBTRACT 2
15
```

## Conclusion and What's Next

잠시 이제까지 우리가 했던 것들을 정리 해 보자.

- 스캐너 (Scanner) 는 언어에서 토큰(Token) 들을 인식하고 리턴한다.
- 파서 (Parser) 는 언어의 문법을 인식하고, Abstract Syntax Tree를 생성한다.
- 연산자 우선순위 테이블을 이용하여 파서는 우리 언어의 의미를 파악할 수 있게 되었다.
- 인터프리터 (Interpreter) 는 AST를 깊이 우선으로 탐색하여 입력 표현식을 계산 할 수 있다.

이를 기반으로 첫 컴파일러의 구현 까지 거의 다 왔다.

다음 장에서는 구현한 인터프리터를 고쳐 AST로부터 x86 기반의 어셈블리 코드를 생성하는 Translator를 작성 할 것이다.
