#pragma once


#include <memory>
#include <string_view>
#include <vector>

#include <mandalang/ir.hpp>
#include <mandalang/type.hpp>


namespace mandalang {

    enum class function_tag {
        builtin, native
    }; // function_tag


    class scope;

    struct function {
        std::string_view name;
        function_tag tag;
        type result;
        std::vector<type> parameters;

        union {
           value (*builtin)(std::vector<value> const&);
           struct native {
               ast_node* body;
               scope* scope;
           } native;
        };

        static function from_builtin(std::string_view name,
                                     type result,
                                     std::vector<type> parameters,
                                     value (*builtin)(std::vector<value> const&)) {
            function r{name, function_tag::builtin, result, std::move(parameters)};
            r.builtin = builtin;
            return r;
        }


        static function from_native(std::string_view name,
                                     type result,
                                     std::vector<type> parameters,
                                     ast_node* body,
                                     scope* scope) {
            function r{name, function_tag::native, result, std::move(parameters)};
            r.native.body = body;
            r.native.scope = scope;
            return r;
        }
    }; // native_function


} // namespace mandalang