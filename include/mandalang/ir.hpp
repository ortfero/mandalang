#pragma once


#include <vector>

#include <configure.hpp>
#include <mandalang/type.hpp>


namespace mandalang {

    enum class ast_node_tag {
        floating_point, integer, name,
        subexpression, resolved_name,
        negate, add, subtract, multiply, divide,
        floating_point_negate, floating_point_add, floating_point_subtract, floating_point_multiply, floating_point_divide,
        integer_negate, integer_add, integer_subtract, integer_multiply, integer_divide,
        boolean_or, boolean_and, boolean_not,
        equals_to, not_equals_to, greater_than, greater_or_equals, less_than, less_or_equals,
        floating_point_equals_to, floating_point_not_equals_to, floating_point_greater_than,
        floating_point_greater_or_equals, floating_point_less_than, floating_point_less_or_equals,
        integer_equals_to, integer_not_equals_to, integer_greater_than, integer_greater_or_equals,
        integer_less_than, integer_less_or_equals,
        boolean_equals_to, boolean_not_equals_to,
        function, typed_name, type_item, resolved_function,
        function_call,
        function_argument, resolved_function_call,
        type_function, type_vector,
        conditional
    };

    struct symbol;
    class scope;

    struct ast_node {
        ast_node_tag tag;
        unsigned line_no;
        union {
            double floating_point;
            platform::integer integer;
            std::string_view name;
            ast_node *unary{nullptr};
            struct binary {
                ast_node *left{nullptr};
                ast_node *right{nullptr};
            } binary;
            struct function {
                unsigned arity;
                ast_node* parameters;
                ast_node* result;
                ast_node* body;
                scope* scope{nullptr};
            } function;
            struct prototype {
                unsigned arity;
                ast_node* parameters;
                ast_node* result;
            } prototype;
            struct call {
                ast_node* callee;
                unsigned arguments_count;
                ast_node* arguments;
            } call;
            struct typed_name {
                ast_node* type;
                std::string_view name;
                ast_node* next;
            } typed_name;
            struct type_item {
                ast_node* type;
                ast_node* next;
            } type_item;
            struct conditional {
                ast_node* condition;
                ast_node* then_branch;
                ast_node* else_branch;
            } conditional;
            symbol const* resolved_name;
        };
        type type;

        ast_node() noexcept { }

        ast_node(ast_node_tag tag, unsigned line_no) noexcept:
            tag{tag}, line_no{line_no} { }

        ast_node(double floating_point, unsigned line_no) noexcept:
            tag{ast_node_tag::floating_point}, line_no{line_no}, floating_point{floating_point} { }

        ast_node(platform::integer integer, unsigned line_no) noexcept:
            tag{ast_node_tag::integer}, line_no{line_no}, integer{integer} { }

        ast_node(ast_node_tag tag, ast_node* unary, unsigned line_no) noexcept:
            tag{tag}, line_no{line_no}, unary{unary} { }

        ast_node(ast_node_tag tag, ast_node* left, ast_node* right, unsigned line_no) noexcept:
            tag{tag}, line_no{line_no}, binary{left, right} { }

        ast_node(std::string_view name, unsigned line_no) noexcept:
            tag{ast_node_tag::name}, line_no{line_no}, name{name} { }

        ast_node(unsigned arity, ast_node* parameters, ast_node* result, ast_node* body, unsigned line_no) noexcept:
            tag{ast_node_tag::function}, line_no{line_no}, function{arity, parameters, result, body} { }

        ast_node(unsigned arity, ast_node* parameters, ast_node* result, unsigned line_no) noexcept:
                tag{ast_node_tag::type_function}, line_no{line_no}, prototype{arity, parameters, result} { }

        ast_node(ast_node* callee, unsigned arguments_count, ast_node* arguments, unsigned line_no) noexcept:
            tag{ast_node_tag::function_call}, line_no{line_no}, call{callee, arguments_count, arguments} { }

        ast_node(ast_node* type, std::string_view name, unsigned line_no) noexcept:
            tag{ast_node_tag::typed_name}, line_no{line_no}, typed_name{type, name, nullptr} { }

        ast_node(ast_node* type, unsigned line_no) noexcept:
            tag{ast_node_tag::type_item}, line_no{line_no}, type_item{type, nullptr} { }

        ast_node(ast_node* condition, ast_node* then_branch, ast_node* else_branch, unsigned line_no) noexcept:
            tag{ast_node_tag::conditional}, line_no{line_no}, conditional{condition, then_branch, else_branch} { }

    }; // ast_node


    struct value;
    struct function_value {
        ast_node* native;
        value (*builtin)(std::vector<value> const&);
    };


    struct value {
        type type;
        union {
            double floating_point;
            platform::integer integer;
            bool boolean;
            function_value function;
        };


        value() noexcept { }

        explicit value(double floating_point) noexcept:
            type{type_tag::floating_point}, floating_point{floating_point} { }

        explicit value(platform::integer integer) noexcept:
            type{type_tag::integer}, integer{integer} { }

        explicit value(bool boolean) noexcept:
            type{type_tag::boolean}, boolean{boolean} { }

        value(struct type const& type, ast_node* native) noexcept:
                type{type}, function{native, nullptr} { }

        value(struct type const& type, value (*builtin)(std::vector<value> const&)) noexcept:
            type{type}, function{nullptr, builtin} { }

        explicit value(struct type const& type) noexcept: type{type} { }

    }; // value


    template<typename S> S& operator << (S& stream, value const& value) {
        switch(value.type.tag) {
            case type_tag::floating_point:
                return stream << value.floating_point;
            case type_tag::integer:
                return stream << value.integer;
            case type_tag::boolean:
                return stream << (value.boolean ? std::string_view{"true"} : std::string_view{"false"});
            case type_tag::composite:
                switch(value.type.composite->tag) {
                    case composite_type_tag::function:
                        return stream << value.type;
                    default:
                        return stream << "unknown";
                }
            default:
                return stream << "<unknown>";
        }
    }


    enum class symbol_tag {
        value, expression, type_expression, type, fn_parameter
    };

    struct symbol {
        std::string_view name;
        symbol_tag tag;
        union {
            value value;
            ast_node* expression;
            struct type type;
            struct function_parameter {
                unsigned index;
                unsigned depth;
                struct type type;
            } function_parameter;
        };

        symbol() noexcept { }

        symbol(std::string_view name, struct value const& value) noexcept:
                name{name}, tag{symbol_tag::value}, value{value} { }

        symbol(std::string_view name, symbol_tag tag, ast_node* expression) noexcept:
            name{name}, tag{tag}, expression{expression} { }

        symbol(std::string_view name, struct type const& type) noexcept:
            name{name}, tag{symbol_tag::type}, type{type} { }

        symbol(std::string_view name, unsigned index, unsigned depth) noexcept:
                name{name}, tag{symbol_tag::fn_parameter}, function_parameter{index, depth} { }
    }; // symbol


    template<typename S> S& operator << (S& stream, symbol const& symbol) {
        stream << symbol.name << " = ";
        switch(symbol.tag) {
            case symbol_tag::value:
                return stream << symbol.value;
            case symbol_tag::expression:
                return stream << "<expression>";
            case symbol_tag::type_expression:
                return stream << "<type expression>";
            case symbol_tag::type:
                return stream << symbol.type;
            case symbol_tag::fn_parameter:
                return stream << symbol.function_parameter.type << " parameter";
            default:
                return stream << "<unknown>";
        }
    }

    enum class symbol_or_expression_tag {
        symbol, expression
    };

    struct symbol_or_expression {
        symbol_or_expression_tag tag;
        union {
            symbol symbol;
            ast_node* expression;
        };

        symbol_or_expression(struct symbol symbol) noexcept: tag{symbol_or_expression_tag::symbol}, symbol{symbol} { }
        symbol_or_expression(struct ast_node* expression) noexcept: tag{symbol_or_expression_tag::expression}, expression{expression} { }
    }; // symbol_or_expression


    enum class symbol_or_value_tag {
        symbol, value
    };


    struct symbol_or_value {
        symbol_or_value_tag tag;
        union {
            symbol const* symbol;
            value value;
        };

        symbol_or_value(struct symbol const* symbol) noexcept: tag{symbol_or_value_tag::symbol}, symbol{symbol} { }
        symbol_or_value(struct value value) noexcept: tag{symbol_or_value_tag::value}, value{value} { }
    };


} // namespace mandalang
