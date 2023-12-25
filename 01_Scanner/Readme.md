# Part 1: Introduction to Lexical Scanning

가장 첫 순서로, 간단한 어휘 스캐너(Lexical Scanner) 부터 만들어 보자. 스캐너는 입력으로부터 _토큰(Token)_ 이라 불리는 어휘 단위를 추출하는 역할을 한다.

일단 다음 다섯 개의 토큰부터 시작해 보자 :

- 4 개의 사칙연산 연산자: `*`, `/`, `+`, `-`
- 1 자리 이상의 숫자로 이루어진 10진수 수

스캐닝 과정에서 생성되는 토큰은 def.hpp 안의 Token 클래스로 표현된다.

```cpp
  class Token {
  public:
    // Types of tokens
    enum class Type {
      T_EOF,
      T_PLUS,
      T_MINUS,
      T_STAR,
      T_SLASH,
      T_INTLIT,
    };

    union Value {
      int int_value_;
    };

    Token(Type type, std::unique_ptr<Value> value = nullptr);
    Type GetType() const;
    template <typename T>
    T GetValue() const;
  private:
    Type type_;
    std::unique_ptr<Value> value_;
  };
```

특히, 해당 토큰의 타입은 아래와 같이 `Type` 타입으로 정의된다.

```cpp
    enum class Type {
      T_EOF,
      T_PLUS,
      T_MINUS,
      T_STAR,
      T_SLASH,
      T_INTLIT,
    };
```

마지막으로, 디버깅 목적으로 각 토큰에 대한 출력을 용이하게 해 주는 `<<` 연산자를 오버로딩했다.

```cpp
std::ostream& operator<<(std::ostream& os, const my_cpp::Token& token) {
  switch (token.GetType()) {
  case my_cpp::Token::Type::T_EOF:
    os << "TOKEN EOF";
    break;
  case my_cpp::Token::Type::T_PLUS:
    os << "TOKEN +";
    break;
  case my_cpp::Token::Type::T_MINUS:
    os << "TOKEN -";
    break;
  case my_cpp::Token::Type::T_STAR:
    os << "TOKEN *";
    break;
  case my_cpp::Token::Type::T_SLASH:
    os << "TOKEN /";
    break;
  case my_cpp::Token::Type::T_INTLIT:
    os << "TOKEN INTLIT(" << token.GetValue<int>() << ")";
    break;
  default:
    throw std::runtime_error("Invalid token type");
  }
  return os;
}
```

## scan.cpp 구현

_scan.cpp_ 파일은 어휘 분석기 구현을 위한 `Scanner` 클래스를 다룬다.

`Scanner` 클래스는 기본적으로 입력 파일 스트림으로부터 한 글자 씩 읽어나간다. 하지만, 종종 입력 스트림으로부터 너무 많은 문자를 읽었을 경우 읽은 글자를 되돌릴 필요가 있다.

> 예를 들어, _23 + 30_ 이라는 문자열에서 토큰을 추출 할 때, _23_ 이라는 숫차를 추출하기 위해서는 다음과 같은 과정을 거친다.
>
> > _2_ : 숫자 (현재 토큰 : 2)  
> > _3_ : 숫자 (현재 토큰: 23)  
> > ' ' : 숫자가 아님 (현재 토큰: 23)
>
> 마지막의 공백(' ')은 다음 토큰을 찾을 때 사용 될 수도 있기 때문에 버려져선 안된다. 이런 경우에 이미 읽은 글자인 공백 (' ') 에서부터 다시 돌아가서 시작 할 (put back) 필요가 있다.

또한 입력 파일에 문제가 있을 때 디버깅 목적으로 문제가 발생한 글자의 위치(라인 번호) 를 기록 해 둘 필요가 있다. 이 모든 일들은 `Scanner` 클래스의 `next()_` 메서드에서 담당한다.

```cpp

  void Scanner::set_put_back(const char c) {
    put_back_ = c;
  }

  char Scanner::get_put_back() const {
    return put_back_;
  }

  char Scanner::next_token()
  {
    char c;
    if (get_put_back() != kNullChar) {
      c = get_put_back();
      set_put_back(kNullChar);
      return c;
    }
    c = input_.get();
    if (c == '\n') {
      line_++;
    }
    return c;
  }
```

## 공백 무시하기

어휘 분석을 효과적으로 하기 위해서는 공백 문자가 나왔을 경우에는 무시를 하도록 할 필요가 있다. `ski()_` 메서드는 이를 담당한다.

```cpp
  char Scanner::skip() {
    char c = next_token();
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      c = next_token();
    }
    return c;
  }
```

## 토큰 스캐닝하기: `scan()`

이제 공백을 무시하면서 문자들을 읽을 준비가 되었다. 또한 이전에 읽었던 문자로 되돌아갈 필요가 있을 경우에도 되돌아 갈 수 있다. 이제 이를 이용해 어휘 분석기(_lexical scanner_) 를 만들어 보자.

```cpp
  std::unique_ptr<Token> Scanner::Scan() {
    char c = skip();
    switch (c) {
    case EOF:
      return std::make_unique<Token>(Token::Type::T_EOF);
    case '+':
      return std::make_unique<Token>(Token::Type::T_PLUS);
    case '-':
      return std::make_unique<Token>(Token::Type::T_MINUS);
    case '*':
      return std::make_unique<Token>(Token::Type::T_STAR);
    case '/':
      return std::make_unique<Token>(Token::Type::T_SLASH);
    default:
      if (is_digit(c)) {
        int n = scan_int(c);
        return std::make_unique<Token>(Token::Type::T_INTLIT, std::make_unique<Token::Value>(Token::Value{ n }));
      }
      else {
        std::stringstream ss;
        ss << "Invalid character : " << c;
        throw std::runtime_error(ss.str());
      }
    }
  }

```

## 정수 리터럴 값 (Integer Literal Values)

`Scan()` 메서드를 보면 정수 값을 읽기 위해 다음과 같은 `is_digit()` 메서드와 `scan_int()` 메서드를 이용하고 있다.

```cpp
  bool Scanner::is_digit(const char c)
  {
    return c >= '0' && c <= '9';
  }

  int Scanner::scan_int(char c) {
    int n = 0;
    while (is_digit(c)) {
      n = 10 * n + (c - '0');
      c = next_token();
    }
    set_put_back(c);
    return n;
  }
```

그리고 다른 연산자와는 다르게 Token 클래스에 대한 객체를 만들 때 얻어낸 정수값을 객채의 값으로 채워준다.

```cpp
      if (is_digit(c)) {
        int n = scan_int(c);
        return std::make_unique<Token>(Token::Type::T_INTLIT, std::make_unique<Token::Value>(Token::Value{ n }));
      }
```

넘겨진 값은 `Token` 클래스의 `value` 필드에 채워진다.

```cpp
    union Value {
      int int_value_;
    };
  private:
    std::unique_ptr<Value> value_;
```

## main() 함수 만들기

이제 `Scanner` 클래스가 완성되었다. main() 함수에서 이 클래스를 이용하는 간단한 프로그램을 만들어 보자.

```cpp
// Project includes
#include "scan.hpp"

// Standard includes
// C++ Standard
#include <fstream>
#include <iostream>

// C Standard

void usage(const char* program_name) {
  std::cerr << "Usage: " << program_name << " <input_file>" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    usage(argv[0]);
    return 1;
  }
  std::ifstream input(argv[1]);
  if (!input) {
    std::cerr << "Failed to open input file" << std::endl;
    return 1;
  }
  auto scanner = my_cpp::utility::MakeScanner(input);
  my_cpp::utility::ScanFile(argv[1]);
  return 0;
}
```

`main()` 함수를 보면 아래와 같은 두 유틸리티 함수를 만들어서 사용하고 있다.

```cpp
    std::unique_ptr<Scanner> MakeScanner(std::istream& input) {
      return std::make_unique<Scanner>(Scanner(input));
    }

    void ScanFile(const std::string& filename) {
      std::ifstream input(filename);
      if (!input) {
        throw std::runtime_error("Cannot open file");
      }
      auto scanner = MakeScanner(input);
      auto token = scanner->Scan();
      while (token->GetType() != Token::Type::T_EOF) {
        std::cout << *token << std::endl;
        token = scanner->Scan();
      }
    }
```

`ScanFile()` 함수는 `MakeScanner()` 함수로 입력 파일에 대한 스캐너를 생성하고, EOF를 만날 때 까지 계속해서 스캔 작업을 하며 토큰을 생성한 후 `cout` 객체에 전달한다.

## 테스트 해 보기

빌드 한 후 테스트 해 보자. 여기서는 간단한 샘플 파일을 만들어 아래와 같이 테스트 해 보았다.

```bash
$ cat samples/input01
2 + 3 * 5 - 8 / 3

$ build/my_cpp samples/input01
TOKEN INTLIT(2)
TOKEN +
TOKEN INTLIT(3)
TOKEN *
TOKEN INTLIT(5)
TOKEN -
TOKEN INTLIT(8)
TOKEN /
TOKEN INTLIT(3)
```

## 마무리

이 장에서는 컴파일러 제작의 첫 순서로 간단한 사칙 연산자와 정수를 받아들이는 어휘 분석기를 만들어 보았다. 이 과정에서 공백 제거 및 읽었던 문자로 되돌아가는 기능도 구현했다.

다음 장에서는 이를 기반으로 간단한 Recursive Descent Parser를 구현해 보고 간단한 Evaluator를 만들어 볼 것이다.
