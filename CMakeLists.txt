cmake_minimum_required(VERSION 3.20)
project(mandalang)

set(CMAKE_CXX_STANDARD 20)

add_executable(mandalang main.cpp
        include/mandalang/loader.hpp
        include/mandalang/mod.hpp
        include/mandalang/scanner.hpp
        include/mandalang/ir.hpp
        include/mandalang/parser.hpp
        include/mandalang/token.hpp
        include/mandalang/error_info.hpp
        include/configure.hpp
        include/mandalang/scope.hpp
        include/mandalang/evaluator.hpp
        include/mandalang/stack_frame.hpp
        include/mandalang/engine.hpp
        include/mandalang/type.hpp
        include/mandalang/type_solver.hpp
        include/mandalang/code_fragment.hpp
        include/mandalang/function.hpp
        include/mandalang/resolver.hpp
        include/mandalang/modules/prelude.hpp)

target_include_directories(mandalang PUBLIC include)
