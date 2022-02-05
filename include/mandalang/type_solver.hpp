#pragma once


#include <nonstd/memory_pool.hpp>
#include <tl/expected.hpp>

#include <mandalang/ir.hpp>
#include <mandalang/error_info.hpp>
#include <mandalang/type.hpp>


namespace mandalang {


    class type_solver {
        nonstd::memory_pool<composite_type>& composite_types_;
        nonstd::memory_pool<symbol>& symbols_;
    public:

        type_solver(nonstd::memory_pool<composite_type>& composite_types,
                    nonstd::memory_pool<symbol>& symbols) noexcept:
            composite_types_{composite_types}, symbols_{symbols} { }


        tl::expected<void, error_info> solve(ast_node* node) noexcept {
            switch(node->tag) {
                case ast_node_tag::floating_point:
                    node->type = type{type_tag::floating_point};
                    return {};
                case ast_node_tag::integer:
                    node->type = type{type_tag::integer};
                    return {};
                case ast_node_tag::resolved_name:
                    return solve_name(node);
                case ast_node_tag::subexpression:
                    return solve_subexpression(node);
                case ast_node_tag::negate:
                    return solve_negate(node);
                case ast_node_tag::boolean_not:
                    return solve_boolean_not(node);
                case ast_node_tag::multiply:
                    return solve_generic_multiply(node);
                case ast_node_tag::divide:
                    return solve_generic_divide(node);
                case ast_node_tag::add:
                    return solve_generic_add(node);
                case ast_node_tag::subtract:
                    return solve_generic_subtract(node);
                case ast_node_tag::boolean_or:
                case ast_node_tag::boolean_and:
                    return solve_boolean_binary(node);
                case ast_node_tag::equals_to:
                    return solve_generic_equals_to(node);
                case ast_node_tag::not_equals_to:
                    return solve_generic_not_equals_to(node);
                case ast_node_tag::greater_than:
                    return solve_generic_greater_than(node);
                case ast_node_tag::greater_or_equals:
                    return solve_generic_greater_or_equals(node);
                case ast_node_tag::less_than:
                    return solve_generic_less_than(node);
                case ast_node_tag::less_or_equals:
                    return solve_generic_less_or_equals(node);
                case ast_node_tag::resolved_function:
                    return solve_function(node);
                case ast_node_tag::resolved_function_call:
                    return solve_function_call(node);
                case ast_node_tag::conditional:
                    return solve_conditional(node);
                default:
                    return failed(error::invalid_ast_node_to_solve_type, node->line_no);
            }
        }

    private:

        tl::expected<void, error_info> solve_name(ast_node* node) noexcept {
            switch(node->resolved_name->tag) {
                case symbol_tag::value:
                    node->type = node->resolved_name->value.type;
                    return {};
                case symbol_tag::expression:
                    node->type = node->resolved_name->expression->type;
                    return {};
                case symbol_tag::type:
                    node->type = node->resolved_name->type;
                    return {};
                case symbol_tag::fn_parameter:
                    node->type = node->resolved_name->function_parameter.type;
                    return {};
                default:
                    return failed(error::invalid_type_resolving, node->line_no);
            }
            return {};
        }


        tl::expected<void, error_info> solve_subexpression(ast_node* node) noexcept {
            auto const solved = solve(node->unary);
            if(!solved)
                return solved;
            node->type = node->unary->type;
            return {};
        }


        tl::expected<void, error_info> solve_negate(ast_node* node) noexcept {
            auto const solved = solve(node->unary);
            if(!solved)
                return solved;
            switch (node->unary->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_negate;
                    node->type = type{type_tag::floating_point};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_negate;
                    node->type = type{type_tag::integer};
                    return {};
                default:
                    return failed(error::unary_minus_should_have_numerical_operand, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_boolean_not(ast_node* node) noexcept {
            auto const solved = solve(node->unary);
            if(!solved)
                return solved;
            if(node->unary->type.tag != type_tag::boolean)
                return failed(error::boolean_not_should_have_boolean_operand, node->line_no);
            node->type.tag = type_tag::boolean;
            return {};
        }


        tl::expected<void, error_info> solve_generic_multiply(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_multiply;
                    node->type = type{type_tag::floating_point};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_multiply;
                    node->type = type{type_tag::integer};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_generic_divide(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_divide;
                    node->type = type{type_tag::floating_point};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_divide;
                    node->type = type{type_tag::integer};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_generic_add(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_add;
                    node->type = type{type_tag::floating_point};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_add;
                    node->type = type{type_tag::integer};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_generic_subtract(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_subtract;
                    node->type = type{type_tag::floating_point};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_subtract;
                    node->type = type{type_tag::integer};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_boolean_binary(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type.tag != type_tag::boolean || node->binary.left->type.tag != type_tag::boolean)
                return failed(error::operands_should_have_boolean_type, node->line_no);
            node->type.tag = type_tag::boolean;
            return {};
        }


        tl::expected<void, error_info> solve_generic_equals_to(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_equals_to;
                    node->type = type{type_tag::boolean};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_equals_to;
                    node->type = type{type_tag::boolean};
                    return {};
                case type_tag::boolean:
                    node->tag = ast_node_tag::boolean_equals_to;
                    node->type = type{type_tag::boolean};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_generic_not_equals_to(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_not_equals_to;
                    node->type = type{type_tag::boolean};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_not_equals_to;
                    node->type = type{type_tag::boolean};
                    return {};
                case type_tag::boolean:
                    node->tag = ast_node_tag::boolean_not_equals_to;
                    node->type = type{type_tag::boolean};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_generic_greater_than(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_greater_than;
                    node->type = type{type_tag::boolean};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_greater_than;
                    node->type = type{type_tag::boolean};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_generic_greater_or_equals(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_greater_or_equals;
                    node->type = type{type_tag::boolean};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_greater_or_equals;
                    node->type = type{type_tag::boolean};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_generic_less_than(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_less_than;
                    node->type = type{type_tag::boolean};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_less_than;
                    node->type = type{type_tag::boolean};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_generic_less_or_equals(ast_node* node) noexcept {
            auto const left_solved = solve(node->binary.left);
            if(!left_solved)
                return left_solved;
            auto const right_solved = solve(node->binary.right);
            if(!right_solved)
                return right_solved;
            if(node->binary.left->type != node->binary.right->type)
                return failed(error::operands_should_have_same_type, node->line_no);
            switch (node->binary.left->type.tag) {
                case type_tag::floating_point:
                    node->tag = ast_node_tag::floating_point_less_or_equals;
                    node->type = type{type_tag::boolean};
                    return {};
                case type_tag::integer:
                    node->tag = ast_node_tag::integer_less_or_equals;
                    node->type = type{type_tag::boolean};
                    return {};
                default:
                    return failed(error::operands_should_have_numerical_types, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_function(ast_node* node) noexcept {
            auto solved = solve_type(node->function.result);
            if(!solved)
                return solved;
            auto i = 0u;
            type parameters[composite_type::max_function_parameters];
            for(auto* current_node = node->function.parameters; current_node != nullptr; current_node = current_node->typed_name.next) {
                solved = solve_type(current_node->typed_name.type);
                if(!solved)
                    return solved;
                auto* parameter_symbol = node->function.scope->find_local(current_node->typed_name.name);
                if(!parameter_symbol)
                    return failed(error::invalid_type_resolving, node->line_no, current_node->typed_name.name);
                parameter_symbol->function_parameter.type = current_node->typed_name.type->type;
                parameters[i++] = current_node->typed_name.type->type;
            }
            auto* function_type = composite_types_.create(node->function.result->type, i, parameters);
            node->type = type{type_tag::composite, function_type};
            auto* self = node->function.scope->find_local("self");
            if(!self)
                return failed(error::invalid_type_resolving, node->line_no);
            self->value.type = node->type;
            solved = solve(node->function.body);
            if(!solved)
                return solved;
            if(node->function.result->type != node->function.body->type)
                return failed(error::mismatch_function_type_and_expression, node->line_no);
            return {};
        }


        tl::expected<void, error_info> solve_function_call(ast_node* node) noexcept {
            auto solved = solve(node->call.callee);
            if(!solved)
                return solved;
            if(node->call.callee->type.tag != type_tag::composite ||
               node->call.callee->type.composite->tag != composite_type_tag::function)
                return failed(error::expected_function_to_call, node->line_no);
            if(node->call.callee->tag == ast_node_tag::resolved_name) {
                switch(node->call.callee->resolved_name->tag) {
                    case symbol_tag::fn_parameter:
                        break;
                    case symbol_tag::value:
                        break;
                    default:
                        return failed(error::expected_function_to_call, node->line_no);
                }
            }
            if(node->call.callee->type.composite->function.arity != node->call.arguments_count)
                return failed(error::mismatch_parameters_and_arguments_count, node->line_no);
            auto parameter_index = 0u;
            for(auto* argument = node->call.arguments; argument != nullptr; argument = argument->binary.right) {
                solved = solve(argument->binary.left);
                if(!solved)
                    return solved;
                if(argument->binary.left->type != node->call.callee->type.composite->function.parameters[parameter_index])
                    return failed(error::mismatch_parameter_and_argument_types, node->line_no);
                ++parameter_index;
            }
            node->type = node->call.callee->type.composite->function.result;
            return {};
        }


        tl::expected<void, error_info> solve_conditional(ast_node* node) noexcept {
            auto solved = solve(node->conditional.condition);
            if(!solved)
                return solved;
            if(node->conditional.condition->type.tag != type_tag::boolean)
                return failed(error::condition_should_be_boolean, node->line_no);
            solved = solve(node->conditional.then_branch);
            if(!solved)
                return solved;
            solved = solve(node->conditional.else_branch);
            if(!solved)
                return solved;
            if(node->conditional.then_branch->type != node->conditional.else_branch->type)
                return failed(error::conditional_expression_types_mismatch, node->line_no);
            node->type = node->conditional.then_branch->type;
            return {};
        }


        tl::expected<void, error_info> solve_type(ast_node* node) noexcept {
            switch(node->tag) {
                case ast_node_tag::resolved_name:
                    if(node->resolved_name->tag != symbol_tag::type)
                        return failed(error::type_name_expected, node->line_no);
                    node->type = node->resolved_name->type;
                    return {};
                case ast_node_tag::type_function:
                    return solve_function_type(node);
                default:
                    return failed(error::invalid_type_syntax, node->line_no);
            }
        }


        tl::expected<void, error_info> solve_function_type(ast_node* node) noexcept {
            auto solved = solve_type(node->prototype.result);
            if(!solved)
                return solved;
            type parameter_types[composite_type::max_function_parameters];
            auto i = 0u;
            for(auto* parameter = node->prototype.parameters; parameter != nullptr; parameter = parameter->type_item.next) {
                solved = solve_type(parameter->type_item.type);
                if (!solved)
                    return solved;
                parameter_types[i++] = parameter->type_item.type->type;
            }
            auto* function_type = composite_types_.create(node->prototype.result->type, i, parameter_types);
            node->type = type{type_tag::composite, function_type};
            return {};
        }
    };

} // namespace mandalang