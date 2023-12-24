cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)


project(my_cpp)

set(SOURCES 
  ast.cpp
  defs.cpp
  gen_x86.cpp
  glob_vars.cpp
  gen.cpp
  parser.cpp
  scan.cpp
  symbols.cpp

  
  main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})