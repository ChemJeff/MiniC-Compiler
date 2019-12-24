cmake_minimum_required(VERSION 3.0)
project(Eeyore2Tigger)

set(OUTPUTS_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUTS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUTS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUTS_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compile_options(-std=c++11 -O2)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

BISON_TARGET(EeyoreParser Eeyore.y ${CMAKE_CURRENT_BINARY_DIR}/Eeyore.y.tab.cc)
FLEX_TARGET(EeyoreLexer Eeyore.lex ${CMAKE_CURRENT_BINARY_DIR}/Eeyore.lex.yy.cc)

ADD_FLEX_BISON_DEPENDENCY(EeyoreLexer EeyoreParser)

# add_definitions(-DPRINTLEX=1)
# add_definitions(-DDEBUGTYPE=1)
# add_definitions(-DDEBUGLIVENESS=1)
# add_definitions(-DDEBUGREG=1)

ADD_EXECUTABLE(Eeyore2Tigger ${BISON_EeyoreParser_OUTPUT_SOURCE} ${FLEX_EeyoreLexer_OUTPUTS} Eeyore.typedef.cc 
Eeyore.liveness.cc Eeyore.regalloc.cc Eeyore2Tigger.cc)
