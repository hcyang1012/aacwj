# Part 6: Variables

이 장에서는 전역변수에 관한 기능을 컴파일러에 추가하였다. 이번 기능은 많은 소스코드들의 수정이 필요하기 떄문에 꽤 긴 여정이 될 것이다.

## What Do We Want from Variables?

이 장에서 우리는 다음과 같은 기능들을 구현 할 것이다.

- 변수 선언
- 변수에 저장되어 있는 값들을 사용
- 변수에 값을 할당

다음은 `samples/input02` 파일에 있는 테스트 프로그램이다.
```C
int fred;
int jim;
fred= 5;
jim= 12;
print fred + jim;
```

가장 눈에 띄는 변화는 문법에 변수 선언과 할당문을 추가했고, 표현식에 변수이름을 사용하도록 한 것이다. 일단 여기서 사용되는 변수를 어떻게 구현 할 지 살펴 보자.

## The Symbol Table

모든 컴파일러들은 심볼 테이블 (Symbol Table) 을 필요로 한다. 나중에는 전역 변수 (Global Variable) 외의 다른 종류의 것들도 다룰 것이다. 일단 심볼 테이블의 각 엔트리 (Entry) 를 살펴보자. (`Symbols.hpp` 에 있다.)

```cpp
class SymbolTableEntry {
public:
    SymbolTableEntry(const std::string &name);
    ~SymbolTableEntry() = default;
    const std::string &GetName() const;

private:
    std::string name_;
};
```

그리고 전역 변수를 위한 심볼 테이블은 `glob_vars.cpp` 에 정의되어 있다.

```cpp
std::vector<SymbolTableEntry> global_symbol_table;
size_t find_global_symbol(const std::string &name) {
    auto it = std::find_if(
            global_symbol_table.begin(),
            global_symbol_table.end(),
            [&name](const SymbolTableEntry &entry) { return entry.GetName() == name; });
    if (it == global_symbol_table.end()) {
        throw std::runtime_error("Symbol not found");
    }
    return std::distance(global_symbol_table.begin(), it);
}
size_t add_global_symbol(const std::string &name) {
    try {
        find_global_symbol(name);
    } catch (std::runtime_error &e) {
        global_symbol_table.emplace_back(name);
    }
    return find_global_symbol(name);
}
```

`global_symbol_table` 벡터는 실질적인 전역변수용 심볼 테이블을 의미한다. 나머지 두 함수들은 아래와 같은 역할을 한다.

- `find_global_symbol` : 찾고자 하는 심볼 이름이 테이블에 있으면 해당 심볼의 인덱스(Index) 를 반환한다.
- `add_global_symbol`: `name` 에 해당하는 심볼이 있는지 확인 후 없으면 새로 추가하고 그 인덱스를 반환한다.

특별히 설명 할 로직은 없다. 이 함수들로 심볼 테이블로부터 심볼을 찾거나 새로 추가 할 수 있게 된다.

## Scanning and New Tokens

앞선 예제 코드를 보면 다음과 같은 토큰이 새로 필요함을 알 수 있다.

- `int` : T_INT 타입의 토큰
- `=` : T_EQUALS 타입의 토큰
- `식별자(Identifier) 명` : T_IDENT 타입의 토큰

`=` 키워드는 `Scanner::Scan()` 메서드에 쉽게 추가 가능하다:
```cpp
    case '=':
        current_ = std::make_unique<Token>(Token::Type::T_EQUALS, std::to_string(c));
        break;
```

`int` 키워드 역시 Scanner::keywords_ 테이블에 토큰을 추가하여 쉽게 추가 할 수 있다: 

```cpp
    const std::map<std::string, Token::Type> keywords_ = {
            {"print", Token::Type::T_PRINT},
            {"int", Token::Type::T_INT},
    };
```

`식별자`를 위해서는 `Scanner::Scan()` 메서드의 마지막 부분을 고칠 필요가 있다: 

```cpp
    default:
        if (is_digit(c)) {
            int n = scan_int(c);
            current_ = std::make_unique<Token>(
                    Token::Type::T_INTLIT, std::to_string(n), std::make_unique<Value>(Value{n}));
        } else if (std::isalpha(c) || '_' == c) {
            std::string s = scan_id(c);
            try {
                Token::Type type = keyword(s);
                current_ = std::make_unique<Token>(type, s);
            } catch (std::runtime_error &e) {
              // Identifier 토큰 추가
                current_ = std::make_unique<Token>(Token::Type::T_IDENT, s);
            }

        } else {
            std::stringstream ss;
            ss << "Invalid character : " << c;
            throw std::runtime_error(ss.str());
        }
```

키워드로 인식되지 않는 문자열은 T_IDENT로 인식하게 하였다.

## The New Grammar

이제 문법이 어떻게 변경되는지 알아보자. 먼저, BNF 표기는 아래와 같이 변경되었다: 

```
 statements: statement
      |      statement statements
      ;

 statement: 'print' expression ';'
      |     'int'   identifier ';'
      |     identifier '=' expression ';'
      ;

 identifier: T_IDENT
      ;
```

식별자(Identifier) 는 `T_IDENT` 토큰을 리턴하도록 되어 있고, `print` 문은 기존에 구현 해 놓은 그대로이다.
하지만 문(Statement)이 세 종류의 가 되었기 때문에 각각을 함수로 묶는게 좋다. 따라서 각각의 경우를 대응하기 위해 `Parser::stmts` 메서드를 다음과 같이 구현했다.

```cpp
std::vector<std::shared_ptr<ASTNode>> Parser::stmts() {
  std::vector<std::shared_ptr<ASTNode>> result;
  while (true) {
    switch (scanner_->Curent().GetType()) {
      case Token::Type::T_PRINT:
        result.push_back(print_stmt());
        break;
      case Token::Type::T_INT:
        result.push_back(var_decl_stmt());
        break;
      case Token::Type::T_IDENT:
        result.push_back(assign_stmt());
        break;
      case Token::Type::T_EOF:
        return result;
      default:
        throw std::runtime_error("Invalid token");
    }
  }
}
```

이전의 `print` 문과 관련된 코드들은 `print_stmt` 메서드로 묶었다. 

## Variable Declarations

이제 변수 선언을 구현 해 보자. `Parser::var_decl_stmt()` 메서드가 이를 담당한다.

```cpp
std::shared_ptr<ASTNode> Parser::var_decl_stmt() {
    match(Token::Type::T_INT);
    const auto kIdentName = scanner_->Curent().GetText();
    ident();
    add_global_symbol(kIdentName);
    semi();
    auto value = std::make_unique<Value>();
    value->sym_id = find_global_symbol(kIdentName);
    auto left = ASTNode::MakeAstLeaf(ASTNode::Type::A_VAR_DECL, std::move(value));
    return left;
}

void Parser::ident() {
    match(Token::Type::T_IDENT);
}
```

일단 `T_INT` 토큰을 발견하면 이 토큰 `T_IDENT` 토큰이 나타나는지 확인하고 이에 대한 텍스트값을 얻는다. 
그리고 이 값을 변수명으로 하여 `add_global_symbol` 메서드를 통해 심볼 테이블에 추가한다. 
다음으로 추가한 심볼에 대한 Symbol Index를 얻은 후 이에 대한 AST 노드를 `A_VAR_DECL` 타입으로 만든다.

## Assignment Statements

할당문은 다음과 같이 Parser::assign_stmt() 에서 구현한다.

```cpp
std::shared_ptr<ASTNode> Parser::assign_stmt() {
    const auto kIdentName = scanner_->Curent().GetText();
    ident();
    auto global_symbol = find_global_symbol(kIdentName);
    auto sym_id = std::make_unique<Value>();
    sym_id->sym_id = global_symbol;
    auto right = ASTNode::MakeAstLeaf(ASTNode::Type::A_LVIDENT, std::move(sym_id));

    match(Token::Type::T_EQUALS);

    auto left = bin_expr();

    auto tree = ASTNode::MakeAstNode(ASTNode::Type::A_ASSIGN, left, right);
    semi();

    return tree;
}
```

일단 몇 가지 AST 노드 타입이 추가되었다. `A_ASSIGN` 타입의 노드는 왼쪽 자식 노드의 표현식 값을 오른쪽 노드의 자식노드에 할당한다. 

오른쪽 자식 노드의 타입은 `A_LVIDENT` 타입이다. 이 타입은 `lvalue` 를 의미한다. `lvalue` 값은 특정 메모리 주소에 위치가 고정되는 값을 의미한다. 예를 들어 아래와 같은 코드가 있을 때:

```cpp
   area = width * height;
```

위 코드는 오른쪽 표현식의 값(`rvalue`) 을 왼쪽의 변수에 할당한다. `rvalue` 는 표현식의 계산값이 아무 레지스터의 위치에 존재하기만 하면 되지만, `area` 변수의 주소는 한번 정해지면 임의로 변경 될 수 없다. 

다만 `rvalue` 표현식을 왼쪽 자식 노드에 먼저 추가한 것을 볼 수 있다. 왼쪽 값을 먼저 계산 후 오른쪽 노드에 할당하는 일관성을 가지게 하기 위해서 이와 같이 구현하였다.

## Changes to the AST Structure

이제 각 AST 노드의 값을 담는 `ASTNode::value_` 필드는 이제 기존의 정수 리터럴 값 뿐 아니라 `A_IDENT` 노드를 위한 심볼 ID도 담을 수 있어야 한다. 이를 위해 `Value` Union을 다음과 같이 수정하였다.

```cpp
union Value {
    int int_value_;
    size_t sym_id; // 새로 추가된 부분
};
```

## Generating the Assignment Code

이제 `CodeGenerator::codegen_ast` 메서드를 살펴보자.

```cpp
size_t CodeGenerator::codegen_ast(const ASTNode &node,
                                  std::optional<size_t> reg) {
  size_t left_reg, right_reg;

  if (node.GetLeft() != nullptr) {
    left_reg = codegen_ast(*node.GetLeft());
  }
  if (node.GetRight() != nullptr) {
    right_reg = codegen_ast(*node.GetRight(), left_reg);
  }

  switch (node.GetOp()) {

    // 중간 생략 

    case ASTNode::Type::A_INTLIT:
      return codegen_load_int(node.GetValue<int>());
    case ASTNode::Type::A_LVIDENT:
      if (!reg.has_value()) {
        throw std::runtime_error("Invalid register");
      }
      return codegen_store_gblob(
          *reg, global_symbol_table.at(node.GetValue<size_t>()).GetName());
    case ASTNode::Type::A_IDENT:
      return codegen_load_gblob(
          global_symbol_table.at(node.GetValue<size_t>()).GetName());
    case ASTNode::Type::A_ASSIGN:
      return right_reg;
    case ASTNode::Type::A_VAR_DECL:
      codegen_symbol(global_symbol_table.at(node.GetValue<size_t>()).GetName());
      return 0;
    case ASTNode::Type::A_PRINT:
      codegen_printint(left_reg);
      return 0;
    default:
      throw std::runtime_error("Invalid ASTNode type");
  }
}
```

먼저 항상 왼쪽 노드의 값을 먼저 계산하고 그 결과값이 저장된 레지스터 번호를 `left_reg` 에 리턴받는다. 
그 다음에 이 레지스터 번호를 오른쪽 트리의 계산에 전달하여 사용하고 있다. 

여기서 이 `left_reg` 는 `A_LVIDENT` 노드의 동작을 위해 쓰인다. 
`codegen_store_gblob` 메서드는 이렇게 전달받은 레지스터 값을 통해 계산된 `rvalue` 값이 어떤 레지스터에 들어가 있는지 알 수 있다.

즉 아래와 같은 AST를 고려해 보면:

```
           A_ASSIGN
          /        \
     A_INTLIT   A_LVIDENT
        (3)        (5)
```

먼저 왼쪽 `A_INTLIT` 노드를 계산하기 위해 `return codegen_load_int(node.GetValue<int>());` 와 같은 코드가 실행이되고, 이 예제에서는 3번 레지스터에 `A_INTLIT` 의 값이 저장된다.

이 3번 레지스터 번호는 `codegen_store_gblob` 메서드에 전달이 되고, 이 예제에서는 5번 ID의 심볼값을 심볼 테이블에서 가져와 그 위치에 3번 레지스에 있는 값을 저장한다.

그리고 다시 `A_ASSIGN` 로 돌아와서, 3번 레지스터의 값을 리턴한다. 이는 다음과 같은 경우를 지원하기 위해서이다:

```cpp
  a = b = c = 0;
```

## Generating x86-64 Code

시작하기 전에, 지난번 `CodeGenerator::codegen_ast` 메서드의 구현과 비교 해 보면 `A_INTLIT` 의 경우에 호출되는 `codegen_load` 메서드의 이름이 `codegen_load_int` 으로 좀 더 구체적으로 변한 것을 알 수 있다.

우선 전역변수의 값을 레지스터로 읽기 위한 코드를 생성하는 `CodeGeneratorX86::codegen_load_gblob` 메서드는 다음과 같이 구현되었다.

```cpp
size_t CodeGeneratorX86::codegen_load_gblob(const std::string &identifier) {
    size_t reg = registers_alloc();
    os_ << "\tmovq\t" << identifier << "(%rip), " << registers_names_[reg] << std::endl;
    return reg;
}
```

반대로 레지스터에 있는 값을 정수 타입 변수에 저장하는 `CodeGeneratorX86::codegen_store_gblob` 메서드는 다음과 같이 구현된다.

```cpp
size_t CodeGeneratorX86::codegen_store_gblob(size_t reg, const std::string &identifier) {
    os_ << "\tmovq\t" << registers_names_[reg] << ", " << identifier << "(%rip)" << std::endl;
    return reg;
}
```

또한, 새로운 전역 변수를 위한 메모리를 할당하는 함수도 다음과 같이 필요하다.

```cpp
void CodeGeneratorX86::codegen_symbol(const std::string &identifier) {
    os_ << "\t.comm\t" << identifier << ",8,8" << std::endl;
}
```

## Variables in Expressions

이제 변수에 값을 할당 할 수 있게 되었다. 그럼 반대로 어떻게 변수의 값을 표현식에 가져와 넣을 수 있을까? 기존에 이미 정수값을 가져오는 `Parser::primary` 메서드가 있기 때문에 이를 약간 수정하여 변수 값도 가져올 수 있도록 해 보자. 자세한 설명은 주석에 표시했다.

```cpp
std::shared_ptr<ASTNode> Parser::primary() {
    const auto &token = scanner_->Curent();
    std::shared_ptr<ASTNode> result = nullptr;
    switch (token.GetType()) {
    case Token::Type::T_INTLIT: {
        auto value = std::make_unique<Value>();
        value->int_value_ = token.GetValue<int>();
        result = ASTNode::MakeAstLeaf(ASTNode::Type::A_INTLIT, std::move(value));
    } break;
    case Token::Type::T_IDENT: {
        // 식별자 (Identifier)가 이미 선언되어 있는지 확인한다.
        auto global_symbol = find_global_symbol(token.GetText());
        // 심볼 ID를 저장 할 새로운 Value 객체를 생성한다.
        auto value = std::make_unique<Value>();
        value->sym_id = global_symbol;

        // 이 Value 객체를 포함하는 새로운 AST Node를 생성한다.
        result = ASTNode::MakeAstLeaf(ASTNode::Type::A_IDENT, std::move(value));
    } break;
    default:
        throw std::runtime_error("Invalid token");
    }
    scanner_->Scan();
    return result;
}
```

## Trying It Out

이제 다음과 같은 입력의 C 코드를 컴파일 해서 실행 해 보자. 
`jim`과 `fred` 라는 이름의 변수에 해당하는 메모리 영역을 할당 후에 이를 기반으로 연산을 수행한 결과를 출력하는 어셈블리 코드를 생성한다.

```
int fred;
int jim;
fred = 5;
jim = 12;
print fred + jim;
```

```bash
$ build/my_cpp samples/input02
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
        .comm   fred,8,8
        .comm   jim,8,8
        movq    $5, %r8
        movq    %r8, fred(%rip)
        movq    $12, %r8
        movq    %r8, jim(%rip)
        movq    fred(%rip), %r8
        movq    jim(%rip), %r9
        addq    %r8, %r9
        movq    %r9, %rdi
        call    printint
        movl    $0, %eax
        popq    %rbp
        ret
$ gcc -o out out.s
$ ./out 
17
```

## Conclusion and What's Next

이번 장에서는 꽤 많은 작업들을 했다. 먼저 심볼 테이블을 만들고 이를 관리하기위한 함수들을 구현했다.
두 종류의 문을 추가했고 관련해서 토큰과 AST 노드 타입도 추가했다. 이를 기반으로 x86-64 어셈블리 코드도 생성했다. 

다음 순서로는 6개의 비교 연산자들을 추가 할 것이다. 이 연산자들은 이후 제어문(Control Statements)들을 구현하기 위한 기본 재료가 될 것이다.