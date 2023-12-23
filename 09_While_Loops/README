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

```
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

```
// scan.cpp
const std::map<std::string, Token::Type> keywords_ = {
    {"print", Token::Type::T_PRINT}, {"int", Token::Type::T_INT},
    {"if", Token::Type::T_IF},       {"else", Token::Type::T_ELSE},
    {"while", Token::Type::T_WHILE}, // 새로 추가된 부분
};
```
