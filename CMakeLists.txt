cmake_minimum_required(VERSION 4.3)
project(nokia2280)

set(CMAKE_CXX_STANDARD 20)

add_executable(nokia_2280 src/main.cpp
        src/upp8m/arm/arm7tdmi.cpp
        src/upp8m/arm/arm7tdmi.h
        src/upp8m/arm/arm7tdmi.h
        src/upp8m/arm/arm7tdmi.h
        src/upp8m/memory.cpp
        src/upp8m/memory.h
        src/upp8m/arm/arm7tdmi.cpp
        src/upp8m/arm/instruction_util.h
        src/upp8m/arm/instruction_util.cpp
        src/toolchain/elf_parser.cpp
        src/toolchain/elf_parser.h)
