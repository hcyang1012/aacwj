# Part 2: Introduction to Parsing

이 장에서는 파서 (Parser) 를 소개하고자 한다. 파서는 입력으로부터 언어의 구문 (_Syntax_) 과 구조적 요소들 (_Structure Elements_) 을 인식하고, 그것들이 문법 (_Grammar_) 를 지키고 있는지 확인하는 역할을 한다. 

앞선 장에서 이미 다음과 같은 토큰 (_Token_) 들을 추출해 냈다.

- 4 개의 사칙연산 연산자: `*`, `/`, `+`, `-`
- 1 자리 이상의 숫자로 이루어진 10진수 수

이제, 이를 기반으로 우리의 파서가 인식 할 언어를 정의 해 보자.

## BNF: Backus-Naur Form

이 프로젝트에서는 BNF 에 대한 자세한 설명은 하지 않고, 만들 언어를 기술하기 위한 정도로만 사용 할 것이다. 자세한 내용은 별도로 찾아보도록 하자.

우선 정수에 대한 사칙연산을 표현하는 문법을 정의 해 보자.

```
expression: number
          | expression '*' expression
          | expression '/' expression
          | expression '+' expression
          | expression '-' expression
          ;

number:  T_INTLIT
         ;
```

세로 선 ( _'|'_ ) 은 문법에서 '또는' 을 의미한다. 즉, 위 문법은 다음과 같다.

- 하나의 표현식은 정수 한개 이거나 
- '*' 토큰으로 구분되는 두 개의 표현식의 묶음이거나 
- '/' 토큰으로 구분되는 두 개의 표현식의 묶음이거나 
- '+' 토큰으로 구분되는 두 개의 표현식의 묶음이거나 
- '-' 토큰으로 구분되는 두 개의 표현식의 묶음이다.
- 정수는 항상 T_INTLIT 토큰으로 정의된다. 

이를 보면 BNF는 재귀적(Recursive) 으로 문법을 정의 할 수 있다는 것을 알 수 있다. 예를 들어, 이 문법에서 `number` 를 제외하면 표현식은 정의 할 때 또 다른 표현식을 가리키고 있다. 

BNF 에서, `expression` 이나 `number` 와 같이 문법 상의 규칙(Rule) 으로서 정의되는 것들을 _non-terminal symbol_ 이라고 부른다.

반면 `T_INTLIT` 은 규칙으로서 정의되는게 아니라 이미 정의되어져 있는 것들이기 때문에 _terminal symbol_ 이라고 부른다. 네 개의 연산자들도 마찬가지로 _terminal symbol_ 이다.

## Recursive Descent Parsing

우리가 정의한 언어가 재귀적이라면, 파싱도 재귀적으로 시도하는 것이 자연스럽다. 이를 위해 해야 할 일은 먼저 토큰을 하나 읽고, 그 이후에 나오는 _토큰도 미리 확인 하는 것_이다. 그 다음 토큰이 어떤 것이냐에 따라 그 다음 파싱 경로(Path)가 결정된다. 이 과정에서 이미 호출했던 파싱 함수는 재귀적으로 한번 더 호출 될 수도 있다.

우리의 경우에는, 가장 첫 토큰은 숫자가 될 것이고, 그 다음 토큰은 사칙연산자가 될 것이다.  그 후에는 숫자가 하나 오거나 또는 새로운 표현식 (Expression)이 올 수 있다. 이 경우 어떻게 재귀적으로 파싱 할 수 있을까? Pseudo-Code는 다음과 같이 구성 할 수 있을 것이다: 

```
function expression() {
  첫 번째 토큰을 읽고, 그 토큰이 숫자인지 확인한다. 그렇지 않으면 에러 리턴.
  그 다음 토큰을 읽는다.
  입력의 끝에 도달했으면, 리턴한다.

  그렇지 않으면 expression() 을 다시 호출한다.
}
```

예제로, `2 + 3 - 5 T_EOF` 입력을 테스트 해 보자. (`T_EOF`는 입력의 끝을 의미한다.)

```
expression0:
  2를 스캔한다. (숫자 2 이다.)
  그 다음 토큰을 읽는다. (T_EOF가 아닌 '+' 이다.)
  expression() 호출

    expression1:
      2를 스캔한다. (숫자 3 이다.)
      그 다음 토큰을 읽는다. (T_EOF가 아닌 '-' 이다.)
      expression() 호출

        expression2:
          5를 스캔한다. (숫자 5 이다.)
          그 다음 토큰을 읽는다. (T_EOF 이다.)
          expression2 에서 리턴한다.

      expression1 에서 리턴한다.
  expression0 에서 리턴한다.
```

`expression()` 함수는 재귀적으로 `2 + 3 - 5 T_EOF` 입력을 파싱했다. 

물론, `expression()` 함수는 이 입력으로 어떠한 것도 하지 않았다. 하지만 그건 파서가 할 일이 일이 아니다. 파서는 단지 입력을 _인식_ 하여 경고나 문법 오류 등을 보여주는 역할을 한다. 또 다른 모듈이 _의미 분석 (Semantic Analysis)_ 을 하는 등의 일을 할 것이다.

> 이후에, 이는 정확한 표현은 아닌 것을 알 수 있게 된다. 종종 구문 분석(Syntax Analysis) 와 의미 분석(Semantic Analysis)를 섞어 부르기도 한다.

## Abstract Syntax Trees

의미 분석을 위해서는, 인식한 입력을 해석(Interpret) 하거나 다른 포맷 (어셈블리어 등) 으로 변경하는 작업을 해야 한다. 이 장에서는 우선 입력에 대한 인터프리터(Interpreter) 를 만들어 볼 것이다. 이를 위해서는 일단 입력을 _Abstract Syntax Tree (AST)_ 로 변경해야 한다. 

> 다음 AST에 대한 짧은 설명글을 읽어보길 추천한다.  
   [Leveling Up One’s Parsing Game With ASTs by Vaidehi Joshi](https://medium.com/basecs/leveling-up-ones-parsing-game-with-asts-d7a6fc2400ff)

AST의 각 노드(Node) 의 구조는 다음과 같이 `ast.hpp` 에 선언되어 있다.

```cpp
class ASTNode {
public:
    // AST node types
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
    // AST structures
    Type op_;
    std::shared_ptr<ASTNode> left_;
    std::shared_ptr<ASTNode> right_;
    std::unique_ptr<Value> value_;
};
```

`A_ADD` 나 `A_SUBSTRACT` 와 같은 연산자를 담고 있는 AST 노드는 `left`, `right` 라는 멤버의 자식 노드 (Child node) 를 가지게 된다. 이후에 이 Sub-tree 값을 더하거나 뺄 때 이것들을 사용하게 된다.

반면 `A_INTLIT` 타입의 노드는 `Value` 멤버에 정수값만 가지고 있을 문 자식 Sub-Tree는 가지고 있지 않다.

## Building AST Nodes and Trees

`ast.cpp` 의 코드들은 `ASTNode` 클래스의 멤버 메서드들을 구현하고 있다. 가장 일반적 경우에 사용되는 `MakeAstNode` 메서드는, `ASTNode` 의 모든 멤버에 들어갈 값들을 인자로 받고, 이를 기반으로 새로운 노드를 생성한다.

```cpp

ASTNode::ASTNode(
        Type op,
        std::shared_ptr<ASTNode> left,
        std::shared_ptr<ASTNode> right,
        std::unique_ptr<Value> value)
    : op_(op), left_(left), right_(right), value_(std::move(value)) {
}

std::shared_ptr<ASTNode> ASTNode::MakeAstNode(
        Type op,
        std::shared_ptr<ASTNode> left,
        std::shared_ptr<ASTNode> right,
        std::unique_ptr<Value> value) {
    return std::make_shared<ASTNode>(op, left, right, std::move(value));
}

```

이를 기반으로, Leaf (Child node가 없는 경우) 나 Child node가 한개만 있는 등 좀 더 구체적인 경우에 사용되는 유틸리티 메서드들을 정의 할 수 있다.

```cpp

std::shared_ptr<ASTNode> ASTNode::MakeAstLeaf(Type op, std::unique_ptr<Value> value) {
    return std::make_shared<ASTNode>(op, nullptr, nullptr, std::move(value));
}

std::shared_ptr<ASTNode> ASTNode::MakeAstUnary(
        Type op,
        std::shared_ptr<ASTNode> left,
        std::unique_ptr<Value> value) {
    return std::make_shared<ASTNode>(op, left, nullptr, std::move(value));
}

```


## Purpose of the AST

이제 AST를 이용해 우리가 인식한 표현식을 메모리에 담거나 표현식의 최종 값을 계산하기 위해 AST를 탐색 (traverse) 하는 방법을 알아보자. 이때, 연산자의 우선순위(Precedence)도 함께 고려해야 할 것이다.

예를 들어, `2 * 3 + 4 * 5` 와 같은 표현식을 생각 해 보면, 곱셈 연산자는 덧셈 연산자보다 우선순위가 더 높기 때문에 곱셈을 먼저 묶어 계산 하고 그 다음에 덧셈을 계산해야 할 것이다.

만약 이에 대한 AST를 아래와 같이 만들었다고 한다면,

```
          +
         / \
        /   \
       /     \
      *       *
     / \     / \
    2   3   4   5
```

AST를 탐색 할 때 `2 * 3` 을 가장 먼저 탐색하고, 그 다음으로 `4 * 5 ` 를 탐색한 후 마지만으로 이 두 값에 대한 결과를 Root 인 덧셈 연산자의 계산에 사용해야 한다. 

## A Naive Expression Parser

우선, 스캐닝 과정에서 생성했던 각 연산자들의 토큰을 AST에서 사용하는 연산자의 형태로 재정의 하자. 물론 앞선 토큰을 그대로 재사용 해도 되지만, 여기서는 좀 더 의미를 확실히 구분하고자 이러한 방법을 택했다. 

이를 위해 `parser.hpp` 에 `Parser` 클래스를 선언했고, 이에 대한 멤버 함수로 `arith_op()` 메서드를 정의했다. 

```cpp

class Parser {
public:
    Parser(std::unique_ptr<Scanner> scanner);
    std::shared_ptr<ASTNode> Parse();

private:
    std::unique_ptr<Scanner> scanner_;

    std::shared_ptr<ASTNode> primary();
    std::shared_ptr<ASTNode> bin_expr();

    ASTNode::Type arith_op(const Token::Type &type) const;
};

ASTNode::Type Parser::arith_op(const Token::Type &type) const {
    switch (type) {
    case Token::Type::T_PLUS:
        return ASTNode::Type::A_ADD;
    case Token::Type::T_MINUS:
        return ASTNode::Type::A_SUBTRACT;
    case Token::Type::T_STAR:
        return ASTNode::Type::A_MULTIPLY;
    case Token::Type::T_SLASH:
        return ASTNode::Type::A_DIVIDE;
    default:
        throw std::runtime_error("Invalid token");
    }
};
```

먼저, 다음 토큰이 정수이면 이 정수값을 담는 AST Node를 생성하는 메서드가 필요하다. 여기서는 `primary()` 메서드가 이를 담당한다.

```cpp
std::shared_ptr<ASTNode> Parser::primary() {
    const auto &token = scanner_->Curent();
    std::shared_ptr<ASTNode> result = nullptr;
    switch (token.GetType()) {
    case Token::Type::T_INTLIT: {
        auto value = std::make_unique<Value>();
        value->int_value_ = token.GetValue<int>();
        result = ASTNode::MakeAstLeaf(ASTNode::Type::A_INTLIT, std::move(value));
        scanner_->Scan();
    } break;
    default:
        throw std::runtime_error("Invalid token");
    }
    return result;
}
```

`Scanner` 클래스의 `Current()` 메서드는 현재 인식된 토큰이 어떤 것인지를 리턴한다. 즉, `next_token()` 메서드는 다음 토큰이 어떤 것인지 확인하여 `Current()` 를 통해 사용자가 알 수 있게 해 준다.

사칙연산자와 같은 Binary Operator에 대해서는 아래와 같은 `bin_expr()` 메서드가 AST Node를 생성한다.

```cpp
std::shared_ptr<ASTNode> Parser::bin_expr() {
    std::shared_ptr<ASTNode> result, left, right;
    left = primary();

    if (scanner_->Curent().GetType() == Token::Type::T_EOF) {
        return left;
    }

    const auto &token = arith_op(scanner_->Curent().GetType());
    scanner_->Scan();
    right = bin_expr();
    result = ASTNode::MakeAstNode(token, left, right);
    return result;
}
```

마지막으로 main() 함수에서는 스캐너에 이어 파서를 호출하여 입력을 파싱한다.


```cpp
  scanner->Scan();
  auto parser = std::make_unique<my_cpp::Parser>(std::move(scanner));
  auto ast = parser->Parse();
```

그러나 `bin_expr()` 과 같은 방식의 파싱은 연산자의 우선순위를 전혀 고려하지 못 하고 있다. 즉, 모든 연산자의 우선순위를 같게 보고 있기 때문에 앞선 예제는 다음과 같은 AST를 만들게 된다.

```
     *
    / \
   2   +
      / \
     3   *
        / \
       4   5
```

이는 틀린 결과라고 할 수 있다. 위 AST를 따라가면, `4 * 5 = 20` 을 먼저 계산 후 `3 + 20 = 23` 및 `2 * 23 = 46` 이라는 결과를 얻게 될 것이다.

## Interpreting the Tree
> 원글에는 이 절이 있지만, 아래 _Code to Interpret the Tree_ 절 만으로 충분히 설명이 가능한 부분이라 생략하였다. 필요하다면 [원글](https://github.com/DoctorWkt/acwj/blob/master/02_Parser/Readme.md#interpreting-the-tree) 을 읽어보자.

## Code to Interpret the Tree

주어진 (_틀린_) AST의 값을 평가(Evaluate) 하기 위해 `Evaluate()` 라는 유틸리티 함수를 ast.cpp 에 구현하였다.

```cpp
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
```

간단히, Left Child 와 Right Child 를 각각 재귀적으로 탐색 하고, Leaf Node인 경우에는 각 연산자에 대한 값을 리턴하는 방식으로 구현하였다. 

## Building the Parser

최종적으로 `main()` 함수에서 AST를 만들고 Evaluate() 함수를 호출하기 위한 코드는 다음과 같다.

```cpp
void usage(const char *program_name) {
    std::cerr << "Usage: " << program_name << " <input_file>" << std::endl;
}

int main(int argc, char *argv[]) {
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
    scanner->Scan();
    auto parser = std::make_unique<my_cpp::Parser>(std::move(scanner));
    auto ast = parser->Parse();
    std::cout << my_cpp::utility::Evaluate(*ast) << std::endl;
    return 0;
}
```

컴파일 후 입력에 대해 테스트를 해 보면 아래와 같다. 확인을 해 보면 연산자들의 우선순위가 올바르지 않게 구현되어 있기 때문에 최종 값이 정상적으로 나오지 않은 것을 알 수 있다.

```bash
$ cat samples/input01
2 + 3 * 5 - 8 / 3

$ build/my_cpp samples/input01
A_INTLIT: 2
A_INTLIT: 3
A_INTLIT: 5
A_INTLIT: 8
A_INTLIT: 3
8 A_DIVIDE 3
5 A_SUBTRACT 2
3 A_MULTIPLY 3
2 A_ADD 9
11 
```

## Conclusion and What's Next

파서는 입력 토큰들을 순서대로 읽어 정의한 언어의 문법을 따르고 있는지 인식하는 역할을 한다. 우리의 언어가 재귀적으로 정의되었기 때문에 우리가 구현한 파서는 재귀적으로 구현되었고, 이를 _Recursive Descent Parser_ 라고 한다. 

앞서 확인 한 대로, 우리가 구현한 파서는 동작은 하지만 입력의 의미(_Semantic_)를 고려하여 파싱하지 않았고, 결과적으로 올바른 값을 계산해 내지는 못 한다. 

다음 장에서는 우리의 파서를 좀 더 수정하여 의미 분석(_Semantic Analysis_) 도 수행하고 올바른 결과값도 얻을 수 있게 할 것이다.