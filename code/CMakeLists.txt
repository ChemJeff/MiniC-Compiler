cmake_minimum_required(VERSION 3.0)
project(MiniC2RISCV32)

add_subdirectory(MiniC2Eeyore)
add_subdirectory(Eeyore2Tigger)
add_subdirectory(Tigger2RISCV32)

file(COPY scripts/MiniC2RISCV32 DESTINATION bin)

