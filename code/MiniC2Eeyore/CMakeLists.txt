cmake_minimum_required(VERSION 3.0)
project(MiniC2Eeyore)

set(OUTPUTS_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUTS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUTS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUTS_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compile_options(-std=c++11 -O2)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

BISON_TARGET(MiniCParser MiniC.y ${CMAKE_CURRENT_BINARY_DIR}/MiniC.y.tab.cc)
FLEX_TARGET(MiniCLexer MiniC.lex ${CMAKE_CURRENT_BINARY_DIR}/MiniC.lex.yy.cc)

ADD_FLEX_BISON_DEPENDENCY(MiniCLexer MiniCParser)

# add_definitions(-DPRINTLEX=1)
# add_definitions(-DPRINTAST=1)
# add_definitions(-DDEBUGAST=1)
# add_definitions(-DDEBUGSYM=1)
# add_definitions(-DDEBUGGENE=1)

ADD_EXECUTABLE(MiniC2Eeyore ${BISON_MiniCParser_OUTPUT_SOURCE} ${FLEX_MiniCLexer_OUTPUTS} MiniC.AST.cc MiniC2Eeyore.cc)
