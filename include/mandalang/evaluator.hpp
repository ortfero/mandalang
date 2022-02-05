#pragma once


#include <vector>

#include <tl/expected.hpp>

#include <mandalang/ir.hpp>
#include <mandalang/error_info.hpp>
#include <mandalang/type_solver.hpp>



namespace mandalang {


    class evaluator {
    private:
        using stack_frame = std::vector<value>;

        std::vector<stack_frame> stack_;

    public:

        tl::expected<value, error_info> evaluate(ast_node* node) noexcept {
            switch(node->tag) {
                case ast_node_tag::floating_point:
                    return {value{node->floating_point}};
                case ast_node_tag::integer:
                    return {value{node->integer}};
                case ast_node_tag::resolved_name:
                    return evaluate_symbol(node);
                case ast_node_tag::integer_negate:
                    return evaluate_integer_negate(node->unary);
                case ast_node_tag::floating_point_negate:
                    return evaluate_floating_point_negate(node->unary);
                case ast_node_tag::boolean_not:
                    return evaluate_boolean_not(node->unary);
                case ast_node_tag::subexpression:
                    return evaluate(node->unary);
                case ast_node_tag::integer_multiply:
                    return evaluate_integer_multiply(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_multiply:
                    return evaluate_floating_point_multiply(node->binary.left, node->binary.right);
                case ast_node_tag::boolean_and:
                    return evaluate_boolean_and(node->binary.left, node->binary.right);
                case ast_node_tag::integer_divide:
                    return evaluate_integer_divide(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_divide:
                    return evaluate_floating_point_divide(node->binary.left, node->binary.right);
                case ast_node_tag::integer_add:
                    return evaluate_integer_add(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_add:
                    return evaluate_floating_point_add(node->binary.left, node->binary.right);
                case ast_node_tag::boolean_or:
                    return evaluate_boolean_or(node->binary.left, node->binary.right);
                case ast_node_tag::integer_subtract:
                    return evaluate_integer_subtract(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_subtract:
                    return evaluate_floating_point_subtract(node->binary.left, node->binary.right);
                case ast_node_tag::integer_equals_to:
                    return evaluate_integer_equals_to(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_equals_to:
                    return evaluate_floating_point_equals_to(node->binary.left, node->binary.right);
                case ast_node_tag::boolean_equals_to:
                    return evaluate_boolean_equals_to(node->binary.left, node->binary.right);
                case ast_node_tag::integer_not_equals_to:
                    return evaluate_integer_not_equals_to(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_not_equals_to:
                    return evaluate_floating_point_not_equals_to(node->binary.left, node->binary.right);
                case ast_node_tag::boolean_not_equals_to:
                    return evaluate_boolean_not_equals_to(node->binary.left, node->binary.right);
                case ast_node_tag::integer_greater_than:
                    return evaluate_integer_greater_than(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_greater_than:
                    return evaluate_floating_point_greater_than(node->binary.left, node->binary.right);
                case ast_node_tag::integer_greater_or_equals:
                    return evaluate_integer_greater_or_equals(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_greater_or_equals:
                    return evaluate_floating_point_greater_or_equals(node->binary.left, node->binary.right);
                case ast_node_tag::integer_less_than:
                    return evaluate_integer_less_than(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_less_than:
                    return evaluate_floating_point_less_than(node->binary.left, node->binary.right);
                case ast_node_tag::integer_less_or_equals:
                    return evaluate_integer_less_or_equals(node->binary.left, node->binary.right);
                case ast_node_tag::floating_point_less_or_equals:
                    return evaluate_floating_point_less_or_equals(node->binary.left, node->binary.right);
                case ast_node_tag::resolved_function:
                    return {value{node->type, node->function.body}};
                case ast_node_tag::resolved_function_call:
                    return evaluate_call(node);
                case ast_node_tag::conditional:
                    return evaluate_conditional(node);
                default:
                    return failed(error::invalid_ast_node_to_evaluate, node->line_no);
            }
        }

    private:


        tl::expected<value, error_info> evaluate_symbol(ast_node* node) {
            switch(node->resolved_name->tag) {
                case symbol_tag::fn_parameter:
                    if(stack_.empty())
                        return failed(error::invalid_stack_operation, node->line_no);
                    return evaluate_function_parameter(node->resolved_name);
                case symbol_tag::expression:
                    return evaluate(node->resolved_name->expression);
                case symbol_tag::value:
                    return {node->resolved_name->value};
                default:
                    return failed(error::invalid_symbol, node->resolved_name->name);
            }
        }


        tl::expected<value, error_info> evaluate_integer_negate(ast_node* node) {
            auto const expected_value = evaluate(node);
            if(!expected_value)
                return expected_value;
            return {value{-expected_value->integer}};
        }


        tl::expected<value, error_info> evaluate_floating_point_negate(ast_node* node) {
            auto const expected_value = evaluate(node);
            if(!expected_value)
                return expected_value;
            return {value{-expected_value->floating_point}};
        }


        tl::expected<value, error_info> evaluate_boolean_not(ast_node* node) {
            auto const expected_value = evaluate(node);
            if(!expected_value)
                return expected_value;
            return {value{!expected_value->boolean}};
        }


        tl::expected<value, error_info> evaluate_integer_multiply(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer * expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_integer_divide(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer / expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_integer_add(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer + expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_integer_subtract(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer - expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_floating_point_multiply(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point * expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_boolean_and(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            if(!expected_left->boolean)
                return {value{false}};
            return evaluate(right);
        }


        tl::expected<value, error_info> evaluate_floating_point_divide(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point / expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_floating_point_add(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point + expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_boolean_or(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            if(expected_left->boolean)
                return {value{true}};
            return evaluate(right);
        }


        tl::expected<value, error_info> evaluate_floating_point_subtract(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point - expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_integer_equals_to(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer == expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_floating_point_equals_to(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point == expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_boolean_equals_to(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->boolean == expected_right->boolean}};
        }


        tl::expected<value, error_info> evaluate_integer_not_equals_to(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer != expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_floating_point_not_equals_to(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point != expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_boolean_not_equals_to(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->boolean != expected_right->boolean}};
        }


        tl::expected<value, error_info> evaluate_integer_greater_than(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer > expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_floating_point_greater_than(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point > expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_integer_greater_or_equals(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer >= expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_floating_point_greater_or_equals(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point >= expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_integer_less_than(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer < expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_floating_point_less_than(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point < expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_integer_less_or_equals(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->integer <= expected_right->integer}};
        }


        tl::expected<value, error_info> evaluate_floating_point_less_or_equals(ast_node* left, ast_node* right) {
            auto const expected_left = evaluate(left);
            if(!expected_left)
                return expected_left;
            auto const expected_right = evaluate(right);
            if(!expected_right)
                return expected_right;
            return {value{expected_left->floating_point <= expected_right->floating_point}};
        }


        tl::expected<value, error_info> evaluate_function_parameter(symbol const* symbol) {
            auto const& parameter = symbol->function_parameter;
            auto const& stack_frame = stack_[stack_.size() - 1 - parameter.depth];
            return {stack_frame[parameter.index]};
        }


        tl::expected<value, error_info> evaluate_call(ast_node* node) {
            auto expected_callee = evaluate_callee(node->call.callee);
            if(!expected_callee)
                return tl::make_unexpected(expected_callee.error());
            if(node->call.arguments_count != 0) {
                auto expected_arguments = evaluate_arguments(node->call.arguments);
                if(!expected_arguments)
                    return tl::make_unexpected(expected_arguments.error());
                if(expected_callee->native) {
                    stack_.emplace_back(std::move(*expected_arguments));
                    auto const expected_result = evaluate(expected_callee->native);
                    stack_.pop_back();
                    return expected_result;
                } else {
                    return expected_callee->builtin(*expected_arguments);
                }
            } else {
                if (expected_callee->native)
                    return evaluate(expected_callee->native);
                else
                    return expected_callee->builtin(std::vector<value>{});
            }
        }


        tl::expected<value, error_info> evaluate_conditional(ast_node* node) {
            auto expected_condition = evaluate(node->conditional.condition);
            if(!expected_condition)
                return expected_condition;
            return evaluate(expected_condition->boolean ? node->conditional.then_branch : node->conditional.else_branch);
        }


        tl::expected<function_value, error_info> evaluate_callee(ast_node* callee) {
            if(callee->tag == ast_node_tag::resolved_name)
                return callee->resolved_name->value.function;
            while(callee != nullptr) {
                switch(callee->tag) {
                    case ast_node_tag::resolved_name:
                        return {callee->resolved_name->value.function};
                    case ast_node_tag::resolved_function:
                        return {function_value{callee->function.body, nullptr}};
                    case ast_node_tag::resolved_function_call:
                        return evaluate_callee_call(callee);
                    case ast_node_tag::subexpression:
                        callee = callee->unary;
                        continue;
                    default:
                        return failed(error::invalid_ast_node_to_evaluate, callee->line_no);
                }
            }
            return failed(error::invalid_ast_node_to_evaluate, callee->line_no);
        }


        tl::expected<function_value, error_info> evaluate_callee_call(ast_node* callee) {
            auto expected_function = evaluate_call(callee);
            if(!expected_function)
                return tl::make_unexpected(expected_function.error());
            return {expected_function->function};
        }


        tl::expected<std::vector<value>, error_info> evaluate_arguments(ast_node* argument) {
            std::vector<value> arguments;
            while(argument != nullptr) {
                auto expected_argument = evaluate(argument->binary.left);
                if(!expected_argument)
                    return tl::make_unexpected(expected_argument.error());
                arguments.emplace_back(std::move(*expected_argument));
                argument = argument->binary.right;
            }
            return {arguments};
        }


    }; // evaluator


} // namespace mandalang
