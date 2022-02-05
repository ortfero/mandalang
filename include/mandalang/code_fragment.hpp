#pragma once


#include <nonstd/memory_pool.hpp>

#include <mandalang/ir.hpp>
#include <mandalang/scope.hpp>
#include <mandalang/type.hpp>


namespace mandalang {


    struct code_fragment {
        std::string source;
        nonstd::memory_pool<composite_type> composite_types;
        nonstd::memory_pool<ast_node> ast;
        nonstd::memory_pool<symbol> symbols;
        nonstd::memory_pool<scope> scopes;
    }; // code_fragment


} // namespace mandalang
