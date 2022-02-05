#pragma once


#include <list>
#include <string>
#include <string_view>
#include <unordered_map>

#include <nonstd/memory_pool.hpp>
#include <tl/expected.hpp>

#include <mandalang/code_fragment.hpp>
#include <mandalang/error_info.hpp>
#include <mandalang/evaluator.hpp>
#include <mandalang/ir.hpp>
#include <mandalang/parser.hpp>
#include <mandalang/resolver.hpp>
#include <mandalang/scope.hpp>
#include <mandalang/type_solver.hpp>


namespace mandalang {


    class mod {
        std::string_view name_;
        std::list<std::unique_ptr<code_fragment>> fragments_;
        nonstd::memory_pool<symbol> common_symbols_;
        scope globals_;
        scope publics_;

    public:
        mod() noexcept = default;
        mod(mod const&) noexcept = default;
        mod& operator = (mod const&) noexcept = default;

        std::string_view const& name() const noexcept { return name_; }
        scope const& publics() const noexcept { return publics_; }

        tl::expected<void, error_info> import(scope const& other) noexcept {
            return globals_.import(other);
        }


        symbol const* redefine(std::string_view name, value const& value) {
            return globals_.redefine(name, value, common_symbols_);
        }


        tl::expected<value, error_info> evaluate_expression(std::string source) {
            auto fragment = std::make_unique<code_fragment>();
            fragment->source = std::move(source);
            parser p{fragment->source.data(), fragment->ast};
            auto const expected_expression = p.parse_expression();
            if(!expected_expression)
                return tl::make_unexpected(expected_expression.error());
            return evaluate_expression(*fragment, *expected_expression);
        }


        tl::expected<symbol_or_value, error_info> evaluate_definition_or_expression(std::string source) {
            auto fragment = std::make_unique<code_fragment>();
            fragment->source = std::move(source);
            parser p{fragment->source.data(), fragment->ast};
            auto const expected_symbol_or_expression = p.parse_definition_or_expression();
            if(!expected_symbol_or_expression)
                return tl::make_unexpected(expected_symbol_or_expression.error());
            if(expected_symbol_or_expression->tag == symbol_or_expression_tag::expression) {
                auto expected_value = evaluate_expression(*fragment, expected_symbol_or_expression->expression);
                if(!expected_value)
                    return tl::make_unexpected(expected_value.error());
                return {std::move(*expected_value)};
            }
            switch(expected_symbol_or_expression->symbol.tag) {
                case symbol_tag::expression:
                    return evaluate_value_definition(std::move(fragment), expected_symbol_or_expression->symbol);
                case symbol_tag::type_expression:
                    return evaluate_type_definition(std::move(fragment), expected_symbol_or_expression->symbol);
                default:
                    return failed(error::invalid_symbol_to_evaluate, expected_symbol_or_expression->symbol.name);
            }
       }

    private:

        tl::expected<value, error_info> evaluate_expression(code_fragment& fragment, ast_node* expression) {
            resolver resolver{fragment.scopes, fragment.symbols};
            auto const resolved = resolver.resolve_expression(globals_, expression);
            if(!resolved)
                return tl::make_unexpected(resolved.error());
            type_solver type_solver{fragment.composite_types, fragment.symbols};
            auto const types_solved = type_solver.solve(expression);
            if(!types_solved)
                return tl::make_unexpected(types_solved.error());
            evaluator evaluator;
            return evaluator.evaluate(expression);
        }


        tl::expected<type, error_info> evaluate_type(code_fragment& fragment, ast_node* expression) {
            resolver resolver{fragment.scopes, fragment.symbols};
            auto const resolved = resolver.resolve_expression(globals_, expression);
            if(!resolved)
                return tl::make_unexpected(resolved.error());
            type_solver type_solver{fragment.composite_types, fragment.symbols};
            auto solved = type_solver.solve(expression);
            if(!solved)
                return tl::make_unexpected(solved.error());
            return expression->type;
        }


        tl::expected<symbol_or_value, error_info> evaluate_value_definition(std::unique_ptr<code_fragment> fragment,
                                                                            symbol const& symbol) {
            auto expected_value = evaluate_expression(*fragment, symbol.expression);
            if(!expected_value)
                return tl::make_unexpected(expected_value.error());
            auto const redefined = globals_.redefine(symbol.name, *expected_value, common_symbols_);
            fragments_.push_front(std::move(fragment));
            return {symbol_or_value{redefined}};
        }


        tl::expected<symbol_or_value, error_info> evaluate_type_definition(std::unique_ptr<code_fragment> fragment,
                                                                           symbol const& symbol) {
            auto expected_type = evaluate_type(*fragment, symbol.expression);
            if(!expected_type)
                return tl::make_unexpected(expected_type.error());
            auto const redefined = globals_.redefine(symbol.name, *expected_type, common_symbols_);
            fragments_.push_front(std::move(fragment));
            return {symbol_or_value{redefined}};
        }

    };

} // mandalang