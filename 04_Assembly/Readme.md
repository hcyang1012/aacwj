# Part 04: An Actual Compiler

이제, 정말로 컴파일러를 만들어 보자. 이 장에서는 이제까지 만들었던 인터프리터를 x86-64 어셈블리어를 생성하는 컴파일러로 바꿔 볼 것이다.

## Revising the Interpreter

시작하기 전에, `ast.cpp` 에 구현 해 둔 `Evaluate()` 함수를 한번 살펴보자.

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

이 함수는 AST를 깊이 우선(Depth-First) 방식 로 탐색하며 왼쪽 Subtree의 값을 계산(Evaluate) 하고, 오른쪽 Subtree의 값을 계산 한 후 `op` 에 해당하는 연산자 대로 다시 한번 이 값들을 계산한다.

만약 `op` 가 사칙연산 중 하나라면 그에 맞는 연산을 수행 할 것이고, 단순히 정수 리터럴(Integer Literal) 이라면 그냥 그 값을 리턴 할 것이다. 최종적으로 이 함수는 마지막으로 입력으로 들어온 AST의 최종 계산 값을 리턴하게 된다.

## Changing to Assembly Code Generation

우선, Target-independent 어셈블리 코드 생성기 (Code generator) 를 만들어 보자. 이 함수는 마지막에는 CPU-Specific 코드 생성함수를 호출하게 될 것이다.

이는 아래와 같이 `gen.cpp ` 에서 `CodeGenerator` 클래스에 구현되어 있다.

```cpp
size_t CodeGenerator::codegen_ast(const ASTNode &node) {
    size_t left_reg, right_reg;

    if (node.GetLeft() != nullptr) {
        left_reg = codegen_ast(*node.GetLeft());
    }
    if (node.GetRight() != nullptr) {
        right_reg = codegen_ast(*node.GetRight());
    }

    switch (node.GetOp()) {
    case ASTNode::Type::A_ADD:
        return codegen_add(left_reg, right_reg);
    case ASTNode::Type::A_SUBTRACT:
        return codegen_sub(left_reg, right_reg);
    case ASTNode::Type::A_MULTIPLY:
        return codegen_mul(left_reg, right_reg);
    case ASTNode::Type::A_DIVIDE:
        return codegen_div(left_reg, right_reg);
    case ASTNode::Type::A_INTLIT:
        return codegen_load(node.GetValue<int>());
    default:
        throw std::runtime_error("Invalid ASTNode type");
    }
}
```

코드를 보면 인터프리터를 구현하기 위한 `Evaluate` 함수와 유사하게 AST를 깊이 우선으로 탐색한다. 그리고 각 연산자들에 대해 다음과 같은 일들을 한다.

- A_INTLIT: 레지스터(Register) 에 정수 리터럴 값을 리턴한다.
- 다른 연산자들: 왼쪽, 오른쪽 Subtree 들의 계산값을 저장하고 있는 레지스터 이름을 파라미터로 받고, 이들을 기반으로 한 사칙연산 함수를 호출한다.

각 `codegen_##` 함수들은 값 자체가 아니라 피연산자들의 값을 담고 있는 레지스터 번호를 기반으로 동작한다. 예를 들어, `codegen_load` 함수는 정수 리터럴 값을 사용하지 않는 레지스터에 읽어들인(load) 후 해당 레지스터 번호를 리턴한다.

`codegen_ast` 메서드 역시 AST의 최종 결과 값을 가지고 있는 레지스터 번호를 리턴한다. 이를 통해 아래와 같이 `codegen_ast` 메서드는 재귀적으로 피연산자들의 레지스터 번호를 얻어낸다.

```cpp
    if (node.GetLeft() != nullptr) {
        left_reg = codegen_ast(*node.GetLeft());
    }
    if (node.GetRight() != nullptr) {
        right_reg = codegen_ast(*node.GetRight());
    }
```

## Calling `codegen_ast`

`codegen_ast` 메서드는 단지 표현식의 값을 계산하는 코드를 생성하는 일만 한다. 따라서 얻어낸 결과 값을 출력하는 것도 추가로 필요하다. 또한 모든 프로그램에 공통적으로 들어가는 초반 코드(_preamble_) 와 후반부 코드(_postamble_) 를 추가하는 메서드도 필요하다. 이러한 작업들은 아래의 `GenerateCode` 에서 수행한다.

```cpp
void CodeGenerator::GenerateCode(const ASTNode &node) {
    codegen_preamble();
    size_t reg = codegen_ast(node);
    codegen_printint(reg);
    codegen_postamble();
}
```

## The x86-64 Code Generator

이제 실제 x86-64 어셈블리 코드를 생성하는 부분을 구현 해 보자. x86-64 어셈블리 코드 생성을 위한 구현은 `gen_x86.cpp` 의 `CodeGeneratorX86` 클래스가 담당한다. 이 클래스는 `CodeGenerator` 클래스를 상속받아 각 코드 생성 메서드들을 구현한다.

## Allocating Registers

어떤 CPU 든 한정된 개수의 레지스터를 가지고 있다. 이 때문에 정수 리터럴 값을 저장하거나 계산 결과 값을 저장하기 위해 레지스터를 할당하고, 더이상 사용되지 않는 값을 담고 있는 레지스터의 할당을 해제하는 메서드가 필요하다. 여기서는 레지스터의 할당, 해제를 위해 다음 세 메서드를 구현하였다.

- registers_free_all(): 모든 레지스터들을 사용 가능한 상태로 해제한다.
- registers_alloc(): 하나의 레지스터를 할당한다.
- registers_free(): 하나의 레지스터의 할당을 해제한다.

이에 대한 구현은 아래와 같다.

```cpp
size_t CodeGeneratorX86::registers_alloc() {
  for (size_t i = 0; i < registers_.size(); ++i) {
    if (!registers_[i]) {
      registers_[i] = true;
      return i;
    }
  }
  throw std::runtime_error("No free registers");
}

void CodeGeneratorX86::registers_free(size_t reg) {
  if (registers_[reg] != true) {
    throw std::runtime_error("Register is not allocated");
  }
  registers_[reg] = false;
}

void CodeGeneratorX86::registers_free_all() {
  for (size_t i = 0; i < registers_.size(); ++i) {
    registers_[i] = false;
  }
}
```

여기서 사용한 할당/해제 로직은 특별한 것이 없어 코드만으로 충분히 이해가 가능한 수준이다. 물론, 모든 레지스터를 다 사용한 상태에서 할당을 요청하면 에러가 발생할 것이다. 이에 대한 대응은 이후에 구현 해 보도록 하자.

여기서는 네 개의 Generic Register를 사용하고 있다. 이에 대응하는 x86-64 레지스터는 다음과 같이 `register_names_` 벡터에 구현하였다.

```cpp
class CodeGeneratorX86 : public CodeGenerator {
 public:
  CodeGeneratorX86(std::ostream &os);
  virtual ~CodeGeneratorX86() = default;

 private:
  std::vector<bool> registers_;
  // 이후 생략
}

CodeGeneratorX86::CodeGeneratorX86(std::ostream &os)
    : CodeGenerator(os),
      registers_(4, false),
      registers_names_{"%r8", "%r9", "%r10", "%r11"} {}

```

이렇게 함으로써 타겟에 종속적이지 않은 코드 생성을 할 수 있다.

### Loading a Register

`codegen_load` 메서드가 이 역할을 담당한다. 먼저 레지스터 하나를 할당하고, movq 명령을 통해 리터럴 값을 할당된 레지스터에 담는다.

```cpp
size_t CodeGeneratorX86::codegen_load(size_t value) {
  size_t reg = registers_alloc();
  os_ << "\tmovq\t$" << value << ", " << registers_names_[reg] << std::endl;
  return reg;
}
```

### Adding two Registers

`codegen_add` 메서드가 이 역할을 담당한다. 이 메서드는 두 개의 레지스터를 인자로 받아 더하는 코드를 생성한다.그 결과는 두 레지스터 중 한 군데에 저장되어 리턴되고, 나머지 하나는 더이상 사용되지 않기 때문에 할당 해제 된다.

```cpp
size_t CodeGeneratorX86::codegen_add(size_t left_reg, size_t right_reg) {
  os_ << "\taddq\t" << registers_names_[left_reg] << ", "
      << registers_names_[right_reg] << std::endl;
  registers_free(left_reg);
  return right_reg;
}
```

### Multiplying Two Registers

`codegen_mul` 메서드가 이를 담당한다. 덧셈과 유사하게 두 레지스터에 대해 곱셈을 수행하고 그 결과물은 둘 중 하나에 저장 후 리턴한다. 나머지 레지스터는 할당 해제된다.

```cpp
size_t CodeGeneratorX86::codegen_mul(size_t left_reg, size_t right_reg) {
  os_ << "\timulq\t" << registers_names_[left_reg] << ", "
      << registers_names_[right_reg] << std::endl;
  registers_free(left_reg);
  return right_reg;
}
```

### Subtracting Two Registers

덧셈, 곱셈과 다르게 뺄셈은 교환 법칙이 성립하지 않는다. 따라서 `codegen_sub` 메서드에서 코드 생성 시에 피연산자의 순서를 고려해 코드를 생성해야 한다. 결과물을 저장 할 레지스터의 리턴 및 해제는 앞선 두 연산과 동일하다.

```cpp
size_t CodeGeneratorX86::codegen_sub(size_t left_reg, size_t right_reg) {
  os_ << "\tsubq\t" << registers_names_[right_reg] << ", "
      << registers_names_[left_reg] << std::endl;
  registers_free(right_reg);
  return left_reg;
}
```

### Dividing Two Registers

나눗셈 또한 교환 법칙이 성립하지 않는다. 그리고 x86-64에서 나눗셈 연산은 다소 복잡하다. 먼저 피제수(dividend) 는 `left` 레지스터로부터 `%rax` 레지스터에 저장한다. 그리고 `rax` 레지스터의 값을 `cqo` 명령으로 8 바이트 정수로 확장한다.

> cqo 명령을 통해 확장하게 되면 값 자체는 변하지 않지만 sign bit 의 위치가 이동하는 등의 효과가 나타난다.

다음으로 `idivq` 명령으로 `%rax` 레지스터 값을 `r2` 레지스터의 값으로 나누고 그 몫을 다시 `%rax` 레지스터에 넣는다. 그리고 마지막으로 그 결과값은 `right_reg` 에 저장하여 리턴하고, 나머지 `left_reg`는 더이상 사용되지 않기 때문에 할당 해제한다.

```cpp
size_t CodeGeneratorX86::codegen_div(size_t left_reg, size_t right_reg) {
  os_ << "\tmovq\t" << registers_names_[left_reg] << ", %rax" << std::endl;
  os_ << "\tcqo" << std::endl;
  os_ << "\tidivq\t" << registers_names_[right_reg] << std::endl;
  os_ << "\tmovq\t%rax, " << registers_names_[right_reg] << std::endl;
  registers_free(left_reg);
  return right_reg;
}
```

### Printing a Register

x86-64 어셈블리어에는 레지스터 값을 10진수로 출력하는 명령이 없다. 따라서 미리 `printint` 라는 함수를 정의하고, 이를 호출하도록 했다. 이 함수의 실제 구현은 `codegen_preamble` 메서드에 있다. 어셈블리어에 대한 자세한 설명은 생략하도록 하겠다.

다만 `codegen_preamble` 메서드는 `main` 함수 의 구현 또한 포함하고 있고, `codegen_postamble` 메서드는 모든 명령이 끝났을 때 `exit` 함수를 호출하는 일 또한 수행한다.

다음은 정수값을 출력하는 `codegen_printint` 메서드의 구현이다.

```cpp
void CodeGeneratorX86::codegen_printint(size_t reg) {
  os_ << "\tmovq\t" << registers_names_[reg] << ", %rdi" << std::endl;
  os_ << "\tcall\tprintint" << std::endl;
  registers_free(reg);
}
```

x86-64 리눅스는 함수의 첫 인자가 `%rdi` 레지스터에 있다고 가정하기 때문에 이 함수는 먼저 인자 값을 `%rdi` 레지스터에 넣고, `printint` 함수를 호출하도록 코드를 생성한다.

## Doing Our First Compile

이제 정말로 컴파일을 해 보고 어셈블리 코드를 생성 해 보자. 그 전에, 코드 생성을 위해서 `main()` 함수가 `GenerateCode` 메서드를 호출하도록 다음처럼 조금 바뀌었다.

```cpp
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

    std::ofstream output("out.s");
    if (!output) {
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
    }
    my_cpp::CodeGeneratorX86 codegen(output);
    codegen.GenerateCode(*ast);
    return 0;
}
```

이를 컴파일 하고 실행하면 다음과 같이 어셈블리어 코드가 `out.s` 라는 파일명으로 생성된다.

```bash
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
        movq    $2, %r8
        movq    $3, %r9
        movq    $5, %r10
        imulq   %r9, %r10
        addq    %r8, %r10
        movq    $8, %r8
        movq    $3, %r9
        movq    %r8, %rax
        cqo
        idivq   %r9
        movq    %rax, %r9
        subq    %r9, %r10
        movq    %r10, %rdi
        call    printint
        movl    $0, %eax
        popq    %rbp
        ret
```

## Examining the Assembly Output

생성된 `out.s` 를 실제 실행파일로 만든 후 실행 해 보자. 정확히 원하는 값을 출력하는 어셈블리 코드를 생성했음을 알 수 있다.

```bash
$ cat samples/input01
2 + 3 * 5 - 8 / 3
$ build/my_cpp samples/input01
$ gcc -o out out.s
$ ./out
15
```

아직은 여전히 어셈블리어로부터 실행 파일을 수동으로 생성해 내야 한다. 이를 자동화 하는 과정도 이후에 살펴 볼 예정이다.

## Conclusion and What's Next

인터프리터에서 코드 생성을 하도록 변경하는 작업은 다소 번거롭지만 각 명령 별로 코드를 생성하는 메서드들을 구현해야 했다. 이를 위해 매우 단순한 레지스터 할당/해제 로직을 구현했으며, 또한 나눗셈을 위한 코드도 약간 추가했다.

한 가지 다루지 않은 부분이 있다면, '왜 파싱 과정에서 바로 코드를 생성하지 않았을까' 하는 점이다. 이 부분에 대해서는 이후 다룰 것이다.

다음 장에서는 다른 언어들 처럼 우리의 언어에 문(Statement) 을 도입 해 볼 것이다.
