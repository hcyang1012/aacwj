cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)


project(my_cpp)

set(SOURCES 
  defs.cpp
  scan.cpp
  main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})