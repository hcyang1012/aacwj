# Part 7: Comparison Operators

이 장에서 `If` 문을 구현하기에 앞서 비교 연산자를 구현해 보고자 한다. 비교 연산자는 이미 앞서 구현 해 본 이항 연사자 (Binary Operator) 의 한 종류이기 때문에 어렵지 않게 구현 할 수 있다.

## Adding New Tokens

여기서는 다음 여섯가지 비교 연산자를 추가 할 것이다.

`==,` `!=`, `<`, `>`, `<=`, `>=`

이를 위해 다음과 같이 `defs.hpp` 에 여섯개의 토큰을 추가하자.

```cpp
  enum Type {
    T_EOF = 0,
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_EQ,  // ==
    T_NE,  // !=
    T_LT,  // <
    T_GT,  // >
    T_LE,  // <=
    T_GE,  // >=
    T_INTLIT,
    T_SEMI,
    T_ASSIGN,
    T_IDENT,

    // Keywords
    T_PRINT,
    T_INT,
  };
```

또한, 앞선 구현과 비교하여 우선순위가 가장 낮은 토큰이 먼저 나타나고 가장 높은 우선순위의 토큰은 가장 뒤에 나타나도록 하되, 우선순위가 없는 토큰은 그 뒤에 오도록 변경하였다.

## Scanning The Tokens

이제 이 토큰들을 인식 해 보도록 하자. 이를 위해서는 스캐너는 다음과 같은 연산자들을 구분할 수 있어야 한다.

- `=` 과 `==`
- `<` 과 `<=`
- `>` 과 `>=`

따라서 스캐너는 첫 글자를 읽은 후 일단 두 번째 글자를 한번 더 읽어 본 후에, 원하는 글자가 아니면 원래대로 되돌려 놓아야 한다. 이를 위해 `scan.cpp` 가 아래와 같이 수정되었다.

```cpp
    case '=':
      if ((c = next_token()) == '=') {
        current_ = std::make_unique<Token>(Token::Type::T_EQ, "==");
      } else {
        set_put_back(c);
        current_ =
            std::make_unique<Token>(Token::Type::T_ASSIGN, std::to_string(c));
      }
      break;
    case '!':
      if ((c = next_token()) == '=') {
        current_ = std::make_unique<Token>(Token::Type::T_NE, "!=");
      } else {
        std::stringstream ss;
        ss << "Invalid character : " << c;
        throw std::runtime_error(ss.str());
      }
      break;
    case '<':
      if ((c = next_token()) == '=') {
        current_ = std::make_unique<Token>(Token::Type::T_LE, "<=");
      } else {
        set_put_back(c);
        current_ = std::make_unique<Token>(Token::Type::T_LT, "<");
      }
      break;
    case '>':
      if ((c = next_token()) == '=') {
        current_ = std::make_unique<Token>(Token::Type::T_GE, ">=");
      } else {
        set_put_back(c);
        current_ = std::make_unique<Token>(Token::Type::T_GT, ">");
      }
      break;
```

또한 `T_EQ`와 구분을 명확히 하기 위해 `=` 토큰의 이름이 `T_ASSIGN` 으로 바뀐 것을 확인 할 수 있다.

## New Expression Code

6개의 토큰을 인식 할 수 있게 되었으니 이제 표현식에서 이 토큰이 있을 때 이를 파싱 할 수 있도록 해 보자. 또한, C 언어에서는 이런 비교 연산자들은 곱셈, 나눗셈 연산자보다 우선순위가 낮기 때문에 이 점도 함께 고려해야 한다.

그리고 점점 토큰의 개수가 많아짐에 따라 이를 파싱하는 `switch` 문의 길이가 길어지는 문제점이 생기고 있기 때문에 이를 줄일 수 있는 방법이 필요해 졌다. 이를 위해 우선 `ast.hpp` 파일의 `ASTNode::Type` 의 값들을 `Token::Type` 의 순서와 동일하게 되도록 재정렬 했다.

```cpp
  enum class Type {
    A_ADD = 1,
    A_SUBTRACT,
    A_MULTIPLY,
    A_DIVIDE,
    A_EQ,
    A_NE,
    A_LT,
    A_GT,
    A_LE,
    A_GE,
    A_INTLIT,
    A_IDENT,
    A_LVIDENT,
    A_ASSIGN,
    A_PRINT,
    A_VAR_DECL,
  };
```

다음으로 `switch` 문으로 구현되어 있던 `Parser::arithop()` 메서드를 다음과 같이 수정했다.

```cpp
ASTNode::Type Parser::arith_op(const Token::Type &type) const {
  if (Token::Type::T_EOF < type && type < Token::Type::T_INTLIT) {
    return static_cast<ASTNode::Type>(type);
  } else {
    throw std::runtime_error("Invalid token");
  }
};
```

마지막으로 연산자 우선순위 테이블도 다음과 같이 수정되었다.

```cpp
    static constexpr size_t kPriority[] = {
            0,   // EOF
            10,  // +
            10,  // -
            20,  // *
            20,  // /
            30,  // ==
            30,  // !=
            40,  // <
            40,  // >
            40,  // <=
            40,  // >=
            0,   // INTLIT
    };
```

## Code Generation

여섯개 연산자 모두 이항 연사자(Binary Operator) 이기 때문에 쉽게 변경을 적용 할 수 있다.
다음과 같이 `CodeGenerator::codegen_ast()` 메서드의 switch 문을 수정하였다.

```cpp
    case ASTNode::Type::A_EQ:
      return codegen_eq(left_reg, right_reg);
    case ASTNode::Type::A_NE:
      return codegen_ne(left_reg, right_reg);
    case ASTNode::Type::A_LT:
      return codegen_lt(left_reg, right_reg);
    case ASTNode::Type::A_GT:
      return codegen_gt(left_reg, right_reg);
    case ASTNode::Type::A_LE:
      return codegen_le(left_reg, right_reg);
    case ASTNode::Type::A_GE:
      return codegen_ge(left_reg, right_reg);
```

## x86-64 Code Generation

[원글](https://github.com/DoctorWkt/acwj/tree/master/07_Comparisons) 에서는 이 부분에 대해 상세히 기술되어 있지만, 여기서는 어셈블리 언어에 대해 다루는 것이 목적은 아니기 때문에 생략하겠다. 상세한 설명은 원글을 참고하자.

## Putting It Into Action

다음과 같은 `samples/input04` 파일을 시험 해 보자.

```
int x;
x= 7 < 9;  print x;
x= 7 <= 9; print x;
x= 7 != 9; print x;
x= 7 == 7; print x;
x= 7 >= 7; print x;
x= 7 <= 7; print x;
x= 9 > 7;  print x;
x= 9 >= 7; print x;
x= 9 != 7; print x;
```

모두 `True` 값을 가지기 때문에 다음과 같은 결과를 볼 수 있다.

```bash
$ build/my_cpp samples/input04
$ gcc -o out out.s
$ ./out
1
1
1
1
1
1
1
1
1
```

## Conclusion and What's Next

이 장에서는 여섯개의 비교 연산자들을 쉽게 추가 해 보았다. 다음장에서는 이를 기반으로 `if` 문을 추가 해 볼 것이다.
