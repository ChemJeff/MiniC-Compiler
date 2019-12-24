cmake_minimum_required(VERSION 3.0)
project(Tigger2RISCV32)

set(OUTPUTS_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUTS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUTS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUTS_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compile_options(-std=c++11 -O2)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

BISON_TARGET(TiggerParser Tigger.y ${CMAKE_CURRENT_BINARY_DIR}/Tigger.y.tab.cc)
FLEX_TARGET(TiggerLexer Tigger.lex ${CMAKE_CURRENT_BINARY_DIR}/Tigger.lex.yy.cc)

ADD_FLEX_BISON_DEPENDENCY(TiggerLexer TiggerParser)

# add_definitions(-DPRINTLEX=1)
# add_definitions(-DDEBUGTYPE=1)

ADD_EXECUTABLE(Tigger2RISCV32 ${BISON_TiggerParser_OUTPUT_SOURCE} ${FLEX_TiggerLexer_OUTPUTS} Tigger.typedef.cc Tigger2RISCV.cc)
