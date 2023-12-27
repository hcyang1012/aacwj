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