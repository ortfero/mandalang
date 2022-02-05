#pragma once


#include <system_error>

#include <tl/expected.hpp>
#include <ufmt/text.hpp>


namespace mandalang {

    enum class error {
        ok,
        not_enough_memory,
        invalid_character,
        invalid_number,
        invalid_expression,
        invalid_operator,
        unclosed_parenthesis_in_expression,
        invalid_ast_node_to_evaluate,
        invalid_ast_node_to_solve_type,
        invalid_ast_node_to_resolve,
        duplicated_name,
        unknown_name,
        unary_minus_should_have_numerical_operand,
        operands_should_have_numerical_types,
        operands_should_have_same_type,
        expected_value_name,
        expected_type_name,
        expected_equals,
        expected_left_parenthesis,
        expected_right_parenthesis,
        expected_arrow,
        name_is_not_found_to_import,
        invalid_type_resolving,
        type_name_expected,
        mismatch_function_type_and_expression,
        expected_parameter_name,
        expected_symbol_name,
        expected_expression_after_function_header,
        expected_argument_or_right_parenthesis,
        expected_comma_or_right_parenthesis,
        expected_function_to_call,
        mismatch_parameters_and_arguments_count,
        mismatch_parameter_and_argument_types,
        invalid_stack_operation,
        invalid_symbol,
        invalid_symbol_to_evaluate,
        invalid_type_syntax,
        boolean_not_should_have_boolean_operand,
        operands_should_have_boolean_type,
        expected_keyword_then,
        expected_keyword_else,
        condition_should_be_boolean,
        conditional_expression_types_mismatch,
        expected_left_square_brace,
        expected_right_square_brace
    }; // error


    class error_category : public std::error_category {
    public:

        char const* name() const noexcept override {
            return "mandalang";
        }

        std::string message(int code) const noexcept override {
            switch(error(code)) {
                case error::ok:
                    return "Ok";
                case error::not_enough_memory:
                    return "Not enough memory";
                case error::invalid_character:
                    return "Invalid character";
                case error::invalid_number:
                    return "Invalid number";
                case error::invalid_expression:
                    return  "Invalid expression";
                case error::invalid_operator:
                    return "Invalid operator";
                case error::unclosed_parenthesis_in_expression:
                    return "Unclosed parenthesis in expression";
                case error::invalid_ast_node_to_evaluate:
                    return "Invalid ast node to evaluate";
                case error::invalid_ast_node_to_solve_type:
                    return "Invalid ast node to solve type";
                case error::invalid_ast_node_to_resolve:
                    return "Invalid ast node to resolve_expression";
                case error::duplicated_name:
                    return "Duplicated name";
                case error::unknown_name:
                    return "Unknown name";
                case error::unary_minus_should_have_numerical_operand:
                    return "Unary minus should have numerical operand";
                case error::operands_should_have_numerical_types:
                    return "Operands should have numerical composite_types";
                case error::operands_should_have_same_type:
                    return "Operands should have same type";
                case error::expected_value_name:
                    return "Expected value name";
                case error::expected_type_name:
                    return "Expected type name";
                case error::expected_equals:
                    return "Expected '='";
                case error::expected_left_parenthesis:
                    return "Expected '('";
                case error::expected_right_parenthesis:
                    return "Expected ')'";
                case error::expected_arrow:
                    return "Expected '->'";
                case error::name_is_not_found_to_import:
                    return "Name is not found to import";
                case error::invalid_type_resolving:
                    return "Invalid type resolving";
                case error::type_name_expected:
                    return "Type name expected";
                case error::mismatch_function_type_and_expression:
                    return "Mismatch function type and expression";
                case error::expected_parameter_name:
                    return "Expected function_parameter name";
                case error::expected_symbol_name:
                    return "Expected resolved_name name";
                case error::expected_expression_after_function_header:
                    return "Expected expression after function header";
                case error::expected_argument_or_right_parenthesis:
                    return "Expected argument or ')'";
                case error::expected_comma_or_right_parenthesis:
                    return "Expected ',' or ')'";
                case error::expected_function_to_call:
                    return "Expected function to call";
                case error::mismatch_parameters_and_arguments_count:
                    return "Mismatch parameters and arguments count";
                case error::mismatch_parameter_and_argument_types:
                    return "Mismatch function parameter and argument composite_types";
                case error::invalid_stack_operation:
                    return "Invalid stack operation";
                case error::invalid_symbol:
                    return "Invalid resolved_name";
                case error::invalid_symbol_to_evaluate:
                    return "Invalid symbol to evaluate";
                case error::invalid_type_syntax:
                    return "Invalid type syntax";
                case error::boolean_not_should_have_boolean_operand:
                    return "'!' should have boolean operand";
                case error::operands_should_have_boolean_type:
                    return "Operands should have boolean type";
                case error::expected_keyword_then:
                    return "Expected 'then'";
                case error::expected_keyword_else:
                    return "Expected 'else'";
                case error::condition_should_be_boolean:
                    return "Expression after 'if' should be boolean";
                case error::conditional_expression_types_mismatch:
                    return "Expressions after 'then' and 'else' should have the same type";
                case error::expected_left_square_brace:
                    return "Expected '['";
                case error::expected_right_square_brace:
                    return "Expected ']'";
                default:
                    return "Unknown";
            }
        }
    }; // error_category


    inline error_category const error_category_instance;

    inline std::error_code make_error_code(error e) noexcept {
        return {int(e), error_category_instance};
    }


    struct error_info {
        std::error_code error_code;
        unsigned line;
        char details[65];
    }; // error_info


    tl::unexpected<error_info> failed(std::error_code error_code) noexcept {
        return tl::make_unexpected(error_info{error_code, 0, '\0'});
    }


    tl::unexpected<error_info> failed(error e) noexcept {
        return tl::make_unexpected(error_info{make_error_code(e), 0, '\0'});
    }


    tl::unexpected<error_info> failed(error e, unsigned line) noexcept {
        return tl::make_unexpected(error_info{make_error_code(e), line, '\0'});
    }


    tl::unexpected<error_info> failed(error e, unsigned line, std::string_view sv) noexcept {
        auto ei = error_info{make_error_code(e), line, '\0'};
        std::size_t n;
        if(sizeof(error_info::details) - 1 < sv.size())
            n = sizeof(error_info::details) - 1;
        else
            n = sv.size();
        std::strncpy(ei.details, sv.data(), n);
        ei.details[n] = '\0';
        return tl::make_unexpected(ei);
    }


    tl::unexpected<error_info> failed(error e, std::string_view sv) noexcept {
        auto ei = error_info{make_error_code(e), 0, '\0'};
        std::size_t n;
        if(sizeof(error_info::details) - 1 < sv.size())
            n = sizeof(error_info::details) - 1;
        else
            n = sv.size();
        std::strncpy(ei.details, sv.data(), n);
        ei.details[n] = '\0';
        return tl::make_unexpected(ei);
    }


    tl::unexpected<error_info> failed(error e, unsigned line, char c) noexcept {
        auto ei = error_info{make_error_code(e), line, '\0'};
        ei.details[0] = c;
        ei.details[1] = '\0';
        return tl::make_unexpected(ei);
    }


    ufmt::text format(error_info const& ei) {
        ufmt::text text;
        if(ei.line != 0)
            text.format("Line ", ei.line, ". ");
        text.format(ei.error_code.message());
        if(ei.details[0] != '\0')
            text.format(" (\'", std::string_view{ei.details}, "\')");
        return text;
    }


    ufmt::text format_without_line(error_info const& ei) {
        ufmt::text text;
        text.format(ei.error_code.message());
        if(ei.details[0] != '\0')
            text.format(" (\'", std::string_view{ei.details}, "\')");
        return text;
    }

} // namespace mandalang


namespace std {
    template <> struct is_error_code_enum<mandalang::error> : true_type {};
}