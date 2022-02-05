#pragma once

#include <string_view>

#include <configure.hpp>


namespace mandalang {

    enum class token_tag {
        floating_point, integer, name,
        plus, minus, asterisk, slash, left_parenthesis, right_parenthesis, left_square_brace, right_square_brace, minus_greater, comma,
        equals, double_equals, exclamation_equals, greater, less, greater_equals, less_equals,
        double_ampersand, double_vertical, exclamation,
        keyword_fn, keyword_let, keyword_type, keyword_if, keyword_then, keyword_else,
        keyword_vector,
        stop
    }; // token_tag

    struct token {
        token_tag tag;
        unsigned line_no;
        union {
            double floating_point;
            platform::integer integer;
            std::string_view name;
        };

        token() noexcept { }

        token(token_tag tag, unsigned line_no) noexcept:
            tag{tag}, line_no{line_no} { }

        token(double floating_point, unsigned line_no) noexcept:
            tag{token_tag::floating_point}, line_no{line_no}, floating_point{floating_point} { }

        token(platform::integer integer, unsigned line_no) noexcept:
            tag{token_tag::integer}, line_no{line_no}, integer{integer} { }

        token(std::string_view name, unsigned line_no) noexcept:
            tag{token_tag::name}, name{name}, line_no{line_no} { }

    };

} // namespace mandalang
