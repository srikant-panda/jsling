cmake_minimum_required(VERSION 3.16)
project(jsling VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(jsling
    src/main.cpp
    src/token.cpp
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    src/environment.cpp
    src/value.cpp
    src/builtins.cpp
    src/interpreter.cpp
    src/errors.cpp
    src/inspect.cpp
    src/cli.cpp
)

target_include_directories(jsling PRIVATE include)

if(MSVC)
    target_compile_options(jsling PRIVATE /W4)
else()
    target_compile_options(jsling PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(NOT MSVC)
    target_link_libraries(jsling PRIVATE m)
endif()

install(TARGETS jsling RUNTIME DESTINATION bin)
