#pragma once


#include <nonstd/memory_pool.hpp>
#include <tl/expected.hpp>

#include <mandalang/ir.hpp>
#include <mandalang/error_info.hpp>
#include <mandalang/scope.hpp>
#include <mandalang/type.hpp>


namespace mandalang {


    class resolver {
        nonstd::memory_pool<scope>& scopes_;
        nonstd::memory_pool<symbol>& symbols_;
    public:

        resolver(nonstd::memory_pool<scope>& scopes, nonstd::memory_pool<symbol>& symbols) noexcept
        : scopes_{scopes}, symbols_{symbols} { }


        tl::expected<void, error_info> resolve_expression(scope& scope, ast_node* node, unsigned depth = 0) {
            switch(node->tag) {
                case ast_node_tag::floating_point:
                case ast_node_tag::integer:
                    return {};
                case ast_node_tag::name:
                    return resolve_name(scope, node);
                case ast_node_tag::subexpression:
                case ast_node_tag::negate:
                case ast_node_tag::boolean_not:
                    return resolve_expression(scope, node->unary);
                case ast_node_tag::multiply:
                case ast_node_tag::divide:
                case ast_node_tag::add:
                case ast_node_tag::subtract:
                case ast_node_tag::boolean_or:
                case ast_node_tag::boolean_and:
                case ast_node_tag::equals_to:
                case ast_node_tag::not_equals_to:
                case ast_node_tag::greater_than:
                case ast_node_tag::greater_or_equals:
                case ast_node_tag::less_than:
                case ast_node_tag::less_or_equals:
                    return resolve_binary_operation(scope, node->binary.left, node->binary.right);
                case ast_node_tag::function:
                    return resolve_function(scope, node);
                case ast_node_tag::function_call:
                    return resolve_function_call(scope, node);
                case ast_node_tag::conditional:
                    return resolve_conditional(scope, node);
                default:
                    return failed(error::invalid_ast_node_to_resolve, node->line_no);
            }
        }


    private:

        tl::expected<void, error_info> resolve_name(scope& scope, ast_node* node) noexcept {
            auto const* symbol_ptr = scope.find(node->name);
            if(!symbol_ptr)
                return failed(error::unknown_name, node->line_no, node->name);
            node->tag = ast_node_tag::resolved_name;
            node->resolved_name = symbol_ptr;
            return {};
        }


        tl::expected<void, error_info> resolve_binary_operation(scope& scope, ast_node* left, ast_node* right) noexcept {
            auto const left_resolved = resolve_expression(scope, left);
            if(!left_resolved)
                return left_resolved;
            auto const right_resolved = resolve_expression(scope, right);
            if(!right_resolved)
                return right_resolved;
            return {};
        }


        tl::expected<void, error_info> resolve_function(scope& scope, ast_node* node) {
            auto resolved = resolve_type(scope, node->function.result);
            if(!resolved)
                return resolved;
            auto* local_scope = scopes_.create(&scope);
            node->function.scope = local_scope;
            auto* self = symbols_.create("self", value{type{}, node->function.body});
            local_scope->define(self);

            auto parameter_index = 0u;
            for(auto* each_parameter = node->function.parameters;
                each_parameter != nullptr;
                each_parameter = each_parameter->typed_name.next) {
                resolved = resolve_type(scope, each_parameter->typed_name.type);
                if(!resolved)
                    return resolved;
                auto* parameter_symbol = symbols_.create(each_parameter->typed_name.name,
                                                         parameter_index, 0);
                auto const defined = local_scope->define(parameter_symbol);
                if(!defined)
                    return tl::make_unexpected(defined.error());
                ++parameter_index;
            }

            node->tag = ast_node_tag::resolved_function;
            return resolve_expression(*local_scope, node->function.body);
        }


        tl::expected<void, error_info> resolve_function_call(scope& scope, ast_node* node) {
            auto resolved = resolve_expression(scope, node->call.callee);
            if(!resolved)
                return resolved;
            for(auto* argument = node->call.arguments; argument != nullptr; argument = argument->binary.right) {
                resolved = resolve_expression(scope, argument->binary.left);
                if(!resolved)
                    return resolved;
            }
            node->tag = ast_node_tag::resolved_function_call;
            return {};
        }


        tl::expected<void, error_info> resolve_conditional(scope& scope, ast_node* node) {
            auto resolved = resolve_expression(scope, node->conditional.condition);
            if(!resolved)
                return resolved;
            resolved = resolve_expression(scope, node->conditional.then_branch);
            if(!resolved)
                return resolved;
            resolved = resolve_expression(scope, node->conditional.else_branch);
            if(!resolved)
                return resolved;
            return {};
        }


        tl::expected<void, error_info> resolve_type(scope& scope, ast_node* node) noexcept {
            switch(node->tag) {
                case ast_node_tag::name:
                    return resolve_type_name(scope, node);
                case ast_node_tag::type_function:
                    return resolve_type_function(scope, node);
                default:
                    return failed(error::invalid_ast_node_to_resolve, node->line_no);
            }
        }


        tl::expected<void, error_info> resolve_type_name(scope& scope, ast_node* node) {
            auto const* symbol_ptr = scope.find(node->name);
            if(!symbol_ptr)
                return failed(error::unknown_name, node->line_no, node->name);
            if(symbol_ptr->tag != symbol_tag::type)
                return failed(error::type_name_expected, node->line_no, node->name);
            node->tag = ast_node_tag::resolved_name;
            node->resolved_name = symbol_ptr;
            return {};
        }


        tl::expected<void, error_info> resolve_type_function(scope& scope, ast_node* node) {
            auto resolved = resolve_type(scope, node->prototype.result);
            if(!resolved)
                return resolved;
            for(auto* parameter = node->prototype.parameters;
                parameter != nullptr;
                parameter = parameter->type_item.next) {
                resolved = resolve_type(scope, parameter->type_item.type);
                if(!resolved)
                    return resolved;
            }
            return {};
        }

    }; // resolver


} // namespace mandalang
