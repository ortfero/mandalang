#pragma once


#include <nonstd/memory_pool.hpp>
#include <tl/expected.hpp>
#include <tl/optional.hpp>

#include <mandalang/ir.hpp>
#include <mandalang/error_info.hpp>
#include <mandalang/scanner.hpp>
#include <mandalang/scope.hpp>



namespace mandalang {


    class parser {
        scanner scanner_;
        nonstd::memory_pool<ast_node>& nodes_;

    public:

        parser(char const* source, nonstd::memory_pool<ast_node>& nodes) noexcept
            : scanner_{source}, nodes_{nodes} { }

        parser(parser const&) noexcept = default;
        parser& operator = (parser const&) noexcept = default;


        tl::expected<symbol_or_expression, error_info> parse_definition_or_expression() {
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto expected_symbol = tl::expected<symbol, error_info>{};
            auto expected_expression = tl::expected<ast_node*, error_info>{};
            switch(expected_token->tag) {
                case token_tag::keyword_let:
                    expected_symbol = parse_value_definition();
                    if(!expected_symbol)
                        return tl::make_unexpected(expected_symbol.error());
                    return {*expected_symbol};
                case token_tag::keyword_type:
                    expected_symbol = parse_type_definition();
                    if(!expected_symbol)
                        return tl::make_unexpected(expected_symbol.error());
                    return {*expected_symbol};
                default:
                    scanner_.back();
                    expected_expression = parse_expression();
                    if(!expected_expression)
                        return tl::make_unexpected(expected_expression.error());
                    return {*expected_expression};
            }
        }


        tl::expected<ast_node*, error_info> parse_expression() {
            auto const expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            switch(expected_token->tag) {
                case token_tag::keyword_fn:
                    return parse_function();
                case token_tag::keyword_if:
                    return parse_conditional();
                default:
                    scanner_.back();
                    return parse_comparison();
            }
        }

    private:


        static tl::optional<ast_node_tag> additive_operator(token_tag tag) noexcept {
            switch(tag) {
            case token_tag::plus:
                return ast_node_tag::add;
            case token_tag::minus:
                return ast_node_tag::subtract;
            default:
                return tl::nullopt;
            }
        }


        static tl::optional<ast_node_tag> multiplicative_operator(token_tag tag) noexcept {
            switch(tag) {
            case token_tag::asterisk:
                return ast_node_tag::multiply;
            case token_tag::slash:
                return ast_node_tag::divide;
            default:
                return tl::nullopt;
            }
        }


        static tl::optional<ast_node_tag> comparison_operator(token_tag tag) noexcept {
            switch(tag) {
                case token_tag::double_equals:
                    return ast_node_tag::equals_to;
                case token_tag::exclamation_equals:
                    return ast_node_tag::not_equals_to;
                case token_tag::greater:
                    return ast_node_tag::greater_than;
                case token_tag::greater_equals:
                    return ast_node_tag::greater_or_equals;
                case token_tag::less:
                    return ast_node_tag::less_than;
                case token_tag::less_equals:
                    return ast_node_tag::less_or_equals;
                default:
                    return tl::nullopt;
            }
        }


        tl::expected<symbol, error_info> parse_value_definition() {
            auto expected_token = scanner_.next(token_tag::name, error::expected_value_name);
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto const value_name = expected_token->name;
            expected_token = scanner_.next(token_tag::equals, error::expected_equals);
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto const expected_expression = parse_expression();
            if(!expected_expression)
                return tl::make_unexpected(expected_expression.error());
            return symbol{value_name, symbol_tag::expression, *expected_expression};
        }


        tl::expected<symbol, error_info> parse_type_definition() {
            auto expected_token = scanner_.next(token_tag::name, error::expected_type_name);
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto const type_name = expected_token->name;
            expected_token = scanner_.next(token_tag::equals, error::expected_equals);
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto const expected_type = parse_type();
            if(!expected_type)
                return tl::make_unexpected(expected_type.error());
            return symbol{type_name, symbol_tag::type_expression, *expected_type};
        }


        tl::expected<ast_node*, error_info> parse_function() {
            auto const expected_header = parse_function_header();
            if(!expected_header)
                return expected_header;
            auto const expected_body = parse_expression();
            if(!expected_body)
                return failed(error::expected_expression_after_function_header, expected_body.error().line);
            auto function_node = nodes_.create((*expected_header)->function.arity,
                                               (*expected_header)->function.parameters,
                                               (*expected_header)->function.result,
                                               *expected_body, (*expected_header)->line_no);
            return {function_node};
        }


        tl::expected<ast_node*, error_info> parse_conditional() {
            auto const line_no = scanner_.line_no();
            auto const expected_condition = parse_expression();
            if(!expected_condition)
                return expected_condition;
            auto expected_token = scanner_.next();
            if(!expected_token || expected_token->tag != token_tag::keyword_then)
                return failed(error::expected_keyword_then, expected_token->line_no);
            auto const expected_then_branch = parse_expression();
            if(!expected_then_branch)
                return expected_then_branch;
            expected_token = scanner_.next();
            if(!expected_token || expected_token->tag != token_tag::keyword_else)
                return failed(error::expected_keyword_else, expected_token->line_no);
            auto const expected_else_branch = parse_expression();
            if(!expected_else_branch)
                return expected_else_branch;
            return {nodes_.create(*expected_condition, *expected_then_branch, *expected_else_branch, line_no)};
        }


        tl::expected<ast_node*, error_info> parse_comparison() {
            auto expected_boolean_term = parse_boolean_term();
            if(!expected_boolean_term)
                return expected_boolean_term;
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto maybe_comparison = comparison_operator(expected_token->tag);
            if (maybe_comparison) {
                    auto another_boolean_term = parse_boolean_term();
                    if(!another_boolean_term)
                        return another_boolean_term;
                    return {nodes_.create(*maybe_comparison, *expected_boolean_term, *another_boolean_term, expected_token->line_no)};
            }
            scanner_.back();
            return expected_boolean_term;
        }


        tl::expected<ast_node*, error_info> parse_boolean_term() {
            auto expected_boolean_factor = parse_boolean_factor();
            if(!expected_boolean_factor)
                return expected_boolean_factor;
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            while(expected_token->tag == token_tag::double_vertical) {
                auto another_boolean_factor = parse_boolean_factor();
                if(!another_boolean_factor)
                    return another_boolean_factor;
                auto operator_node = nodes_.create(ast_node_tag::boolean_or, *expected_boolean_factor, *another_boolean_factor, expected_token->line_no);
                expected_boolean_factor = operator_node;
                expected_token = scanner_.next();
                if(!expected_token)
                    return tl::make_unexpected(expected_token.error());
            }
            scanner_.back();
            return expected_boolean_factor;
        }


        tl::expected<ast_node*, error_info> parse_boolean_factor() {
            auto expected_term = parse_term();
            if(!expected_term)
                return expected_term;
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            while(expected_token->tag == token_tag::double_ampersand) {
                auto another_term = parse_term();
                if(!another_term)
                    return another_term;
                auto operator_node = nodes_.create(ast_node_tag::boolean_and, *expected_term, *another_term, expected_token->line_no);
                expected_term = operator_node;
                expected_token = scanner_.next();
                if(!expected_token)
                    return tl::make_unexpected(expected_token.error());
            }
            scanner_.back();
            return expected_term;
        }


        tl::expected<ast_node*, error_info> parse_term() {
            auto expected_term = parse_factor();
            if(!expected_term)
                return expected_term;
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto maybe_additive = additive_operator(expected_token->tag);
            while(maybe_additive) {
                auto another_expected_term = parse_factor();
                if(!another_expected_term)
                    return another_expected_term;
                auto operator_node = nodes_.create(*maybe_additive, *expected_term, *another_expected_term, expected_token->line_no);
                expected_term = operator_node;
                expected_token = scanner_.next();
                if(!expected_token)
                    return tl::make_unexpected(expected_token.error());
                maybe_additive = additive_operator(expected_token->tag);
            }
            scanner_.back();
            return expected_term;
        }


        tl::expected<ast_node*, error_info> parse_function_header() {
            auto expected_token = scanner_.next(token_tag::left_parenthesis, error::expected_left_parenthesis);
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto const line_no = expected_token->line_no;
            auto arity = 0u;
            auto parameters = (ast_node*)nullptr;

            expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());

            if(expected_token->tag != token_tag::right_parenthesis) {
                scanner_.back();
                auto expected_parameters = parse_typed_names(arity);
                if(!expected_parameters)
                    return expected_parameters;
                parameters = *expected_parameters;
                expected_token = scanner_.next();
                if(!expected_token)
                    return tl::make_unexpected(expected_token.error());
                if(expected_token->tag != token_tag::right_parenthesis)
                    return failed(error::expected_right_parenthesis, expected_token->line_no);
            }

            expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            if(expected_token->tag != token_tag::minus_greater)
                return failed(error::expected_arrow, expected_token->line_no);

            auto expected_result = parse_type();
            if(!expected_result)
                return expected_result;

            return {nodes_.create(arity, parameters, *expected_result, nullptr, line_no)};
        }


        tl::expected<ast_node*, error_info> parse_factor() {
            auto expected_factor = parse_unary();
            if(!expected_factor)
                return expected_factor;
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto maybe_multiplicative = multiplicative_operator(expected_token->tag);
            while(maybe_multiplicative) {
                auto another_expected_factor = parse_unary();
                if(!another_expected_factor)
                    return another_expected_factor;
                auto operator_node = nodes_.create(*maybe_multiplicative, *expected_factor, *another_expected_factor, expected_token->line_no);
                expected_factor = operator_node;
                expected_token = scanner_.next();
                if(!expected_token)
                    return tl::make_unexpected(expected_token.error());
                maybe_multiplicative = multiplicative_operator(expected_token->tag);
            }
            scanner_.back();
            return expected_factor;
        }


        tl::expected<ast_node*, error_info> parse_typed_names(unsigned& count) {
            count = 0;
            auto expected_typed_name = parse_typed_name();
            if(!expected_typed_name)
                return expected_typed_name;
            ++count;
            auto first_typed_name = *expected_typed_name;
            auto last_typed_name = first_typed_name;
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            while(expected_token->tag == token_tag::comma) {
                expected_typed_name = parse_typed_name();
                if(!expected_typed_name)
                    return expected_typed_name;
                ++count;
                last_typed_name->typed_name.next = *expected_typed_name;
                last_typed_name = *expected_typed_name;
            }
            scanner_.back();
            return first_typed_name;
        }


        tl::expected<ast_node*, error_info> parse_type() {
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto expected_type = tl::expected<ast_node*, error_info>{};
            switch(expected_token->tag) {
                case token_tag::name:
                    return {nodes_.create(expected_token->name, expected_token->line_no)};
                case token_tag::left_parenthesis:
                    expected_type = parse_type();
                    if(!expected_type)
                        return expected_type;
                    expected_token = scanner_.next(token_tag::right_parenthesis, error::unclosed_parenthesis_in_expression);
                    if(!expected_token)
                        return tl::make_unexpected(expected_token.error());
                    return expected_type;
                case token_tag::keyword_fn:
                    return parse_function_type();
                case token_tag::keyword_vector:
                    return parse_vector_type();
                default:
                    return failed(error::invalid_type_syntax, expected_token->line_no);
            }
        }


        tl::expected<ast_node*, error_info> parse_unary() {
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            switch(expected_token->tag) {
                case token_tag::plus:
                    return parse_unary();
                case token_tag::minus:
                    return parse_unary_operator(ast_node_tag::negate);
                case token_tag::exclamation:
                    return parse_unary_operator(ast_node_tag::boolean_not);
                case token_tag::left_parenthesis:
                    return parse_subexpression_and_optional_calls();
                case token_tag::floating_point:
                    return parse_floating_point(*expected_token);
                case token_tag::integer:
                    return parse_integer(*expected_token);
                case token_tag::name:
                    return parse_name_and_optional_calls(*expected_token);
                default:
                    return failed(error::invalid_expression, scanner_.line_no());
            }
        }


        tl::expected<ast_node*, error_info> parse_typed_name() {
            auto expected_type = parse_type();
            if(!expected_type)
                return expected_type;
            auto expected_name = scanner_.next(token_tag::name, error::expected_parameter_name);
            if(!expected_name)
                return tl::make_unexpected(expected_name.error());
            auto typed_name_node = nodes_.create(*expected_type, expected_name->name, expected_name->line_no);
            return {typed_name_node};
        }


        tl::expected<ast_node*, error_info> parse_function_type() {
            auto expected_token = scanner_.next(token_tag::left_parenthesis, error::expected_left_parenthesis);
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto const line_no = expected_token->line_no;
            unsigned arity = 0;
            auto parameters = (ast_node*)nullptr;
            expected_token = scanner_.next();
            if(expected_token->tag != token_tag::right_parenthesis) {
                scanner_.back();
                auto expected_type = parse_type();
                if(!expected_type)
                    return expected_type;
                ++arity;
                parameters = nodes_.create(*expected_type, (*expected_type)->line_no);
                auto last_parameter = parameters;
                expected_token = scanner_.next();
                if(!expected_token)
                    return tl::make_unexpected(expected_token.error());
                while(expected_token->tag == token_tag::comma) {
                    expected_type = parse_type();
                    if(!expected_type)
                        return expected_type;
                    ++arity;
                    last_parameter->type_item.next = nodes_.create(*expected_type, (*expected_type)->line_no);
                    last_parameter = last_parameter->type_item.next;
                    expected_token = scanner_.next();
                    if(!expected_token)
                        return tl::make_unexpected(expected_token.error());
                }
                if(expected_token->tag != token_tag::right_parenthesis)
                    return failed(error::expected_comma_or_right_parenthesis, expected_token->line_no);
            }
            expected_token = scanner_.next(token_tag::minus_greater, error::expected_arrow);
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto expected_result = parse_type();
            if(!expected_result)
                return expected_result;
            return {nodes_.create(arity, parameters, *expected_result, line_no)};
        }


        tl::expected<ast_node*, error_info> parse_vector_type() {
            auto expected_token = scanner_.next(token_tag::left_square_brace, error::expected_left_square_brace);
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            auto const line_no = expected_token->line_no;
            auto expected_type = parse_type();
            if(!expected_type)
                return expected_type;
            expected_token = scanner_.next(token_tag::right_square_brace, error::expected_right_square_brace);
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            return {nodes_.create(ast_node_tag::type_vector, *expected_type, line_no)};
        }


        tl::expected<ast_node*, error_info> parse_unary_operator(ast_node_tag tag) {
            auto const line_no = scanner_.line_no();
            auto expected_factor = parse_unary();
            if(!expected_factor)
                return expected_factor;
            auto negative_node = nodes_.create(tag, *expected_factor, line_no);
            return {negative_node};
        }


        tl::expected<ast_node*, error_info> parse_subexpression_and_optional_calls() {
            auto const expected_node = parse_subexpression();
            if(!expected_node)
                return expected_node;
            return parse_optional_calls(*expected_node);
        }


        tl::expected<ast_node*, error_info> parse_subexpression() {
            auto const line_no = scanner_.line_no();
            auto expected_node = parse_expression();
            if(!expected_node)
                return expected_node;
            auto const expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            if(expected_token->tag != token_tag::right_parenthesis)
                return failed(error::unclosed_parenthesis_in_expression, scanner_.line_no());
            auto subexpression_node = nodes_.create(ast_node_tag::subexpression, *expected_node, line_no);
            return {subexpression_node};
        }


        tl::expected<ast_node*, error_info> parse_floating_point(token token) {
            return {nodes_.create(token.floating_point, token.line_no)};
        }


        tl::expected<ast_node*, error_info> parse_integer(token token) {
            return {nodes_.create(token.integer, token.line_no)};
        }


        tl::expected<ast_node*, error_info> parse_name_and_optional_calls(token token) {
            auto const expected_node = parse_name(token);
            if(!expected_node)
                return expected_node;
            return parse_optional_calls(*expected_node);
        }


        tl::expected<ast_node*, error_info> parse_name(token token) {
            return {nodes_.create(token.name, token.line_no)};
        }


        tl::expected<ast_node*, error_info> parse_optional_calls(ast_node* node) {
            ast_node* call_node = node;
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            while(expected_token->tag == token_tag::left_parenthesis) {
                unsigned arguments_count;
                auto expected_arguments = parse_arguments(arguments_count);
                if(!expected_arguments)
                    return expected_arguments;
                call_node = nodes_.create(call_node, arguments_count, *expected_arguments, call_node->line_no);
                expected_token = scanner_.next();
                if(!expected_token)
                    return tl::make_unexpected(expected_token.error());
            }
            scanner_.back();
            return {call_node};
        }


        tl::expected<ast_node*, error_info> parse_arguments(unsigned& arguments_count) {
            arguments_count = 0;
            auto expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            if(expected_token->tag == token_tag::right_parenthesis)
                return nullptr;
            scanner_.back();
            auto expected_expression = parse_expression();
            if(!expected_expression)
                return failed(error::expected_argument_or_right_parenthesis, expected_token->line_no);
            auto* arguments_node = nodes_.create(ast_node_tag::function_argument, *expected_expression, nullptr, (*expected_expression)->line_no);
            auto* current_argument = arguments_node;
            ++arguments_count;
            expected_token = scanner_.next();
            if(!expected_token)
                return tl::make_unexpected(expected_token.error());
            while(expected_token->tag != token_tag::right_parenthesis) {
                if(expected_token->tag != token_tag::comma)
                    return failed(error::expected_comma_or_right_parenthesis, expected_token->line_no);
                auto expected_expression = parse_expression();
                if(!expected_expression)
                    return failed(error::expected_argument_or_right_parenthesis, expected_token->line_no);
                auto* argument_node = nodes_.create(ast_node_tag::function_argument, *expected_expression, nullptr, (*expected_expression)->line_no);
                current_argument->binary.right = argument_node;
                current_argument = argument_node;
                ++arguments_count;
                expected_token = scanner_.next();
                if(!expected_token)
                    return tl::make_unexpected(expected_token.error());
            }
            return {arguments_node};
        }

    }; // parser

} // namespace mandalang