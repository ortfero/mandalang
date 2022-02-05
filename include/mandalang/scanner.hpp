#pragma once


#include <charconv>

#include <tl/expected.hpp>
#include <tl/optional.hpp>

#include <configure.hpp>
#include <mandalang/error_info.hpp>
#include <mandalang/token.hpp>


namespace mandalang {

    class scanner {
        char const* p_{nullptr};
        unsigned line_no_{1};
        bool backward_{false};
        token token_{};
    public:

        scanner(char const* p) noexcept: p_{p} { }
        scanner(scanner const&) noexcept = default;
        scanner& operator = (scanner const&) noexcept = default;

        unsigned line_no() const noexcept { return line_no_; }
        char const* position() const noexcept { return p_; }
        void position(char const* p) noexcept { p_ = p; }


        tl::expected<token, error_info> next() noexcept {
            if(backward_) {
                backward_ = false;
                return token_;
            }
            auto const expected_token = scan();
            if(!expected_token)
                return expected_token;
            token_ = *expected_token;
            return expected_token;
        }


        tl::expected<token, error_info> next(token_tag tag, error e) noexcept {
            auto const token = next();
            if(!token)
                return token;
            if(token->tag != tag)
                return failed(e, token->line_no);
            return token;
        }


        void back() {
            backward_ = true;
        }


    private:

        tl::expected<token, error_info> scan() noexcept {
            // clang-format off
            auto const maybe_token = skip_spaces_and_comments();
            if(maybe_token)
                return *maybe_token;
            switch(*p_) {
                case '(':
                    ++p_;
                    return {token{token_tag::left_parenthesis, line_no_}};
                case ')':
                    ++p_;
                    return {token{token_tag::right_parenthesis, line_no_}};
                case '[':
                    ++p_;
                    return {token{token_tag::left_square_brace, line_no_}};
                case ']':
                    ++p_;
                    return {token{token_tag::right_square_brace, line_no_}};
                case '+':
                    ++p_;
                    return {token{token_tag::plus, line_no_}};
                case '-':
                    ++p_;
                    if(*p_ == '>') {
                        ++p_;
                        return {token{token_tag::minus_greater, line_no_}};
                    }
                    return {token{token_tag::minus, line_no_}};
                case '*':
                    ++p_;
                    return {token{token_tag::asterisk, line_no_}};
                case '/':
                    ++p_;
                    return { token{token_tag::slash, line_no_}};
                case ',':
                    ++p_;
                    return { token{token_tag::comma, line_no_}};
                case '=':
                    ++p_;
                    if(*p_ == '=') {
                        ++p_;
                        return {token{token_tag::double_equals, line_no_}};
                    }
                    return { token{token_tag::equals, line_no_}};
                case '!':
                    ++p_;
                    if(*p_ == '=') {
                        ++p_;
                        return {token{token_tag::exclamation_equals, line_no_}};
                    }
                    return {token{token_tag::exclamation, line_no_}};
                case '>':
                    ++p_;
                    if(*p_ == '=') {
                        ++p_;
                        return {token{token_tag::greater_equals, line_no_}};
                    }
                    return {token{token_tag::greater, line_no_}};
                case '<':
                    ++p_;
                    if(*p_ == '=') {
                        ++p_;
                        return {token{token_tag::less_equals, line_no_}};
                    }
                    return {token{token_tag::less, line_no_}};
                case '&':
                    ++p_;
                    if(*p_ != '&')
                        return failed(error::invalid_operator, line_no_, '&');
                    ++p_;
                    return {token{token_tag::double_ampersand, line_no_}};
                case '|':
                    ++p_;
                    if(*p_ != '|')
                        return failed(error::invalid_operator, line_no_, '|');
                    ++p_;
                    return {token{token_tag::double_vertical, line_no_}};
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    return scan_number();
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
                case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
                case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': case 'a': case 'b': case 'c':
                case 'g': case 'h': case 'j': case 'k': case 'm': case 'n':
                case 'o': case 'p': case 'q': case 'r': case 's': case 'u': case 'w': case 'x':
                case 'y': case 'z': case '_':
                    return scan_name();
                case 'd':
                    //if(try_scan_keyword('o', 'u', 'b', 'l', 'e'))
                    //    return {token{token_tag::keyword_double, line_no_}};
                    return scan_name();
                case 'e':
                    if(try_scan_keyword('l', 's', 'e'))
                        return {token{token_tag::keyword_else, line_no_}};
                    return scan_name();
                case 'f':
                    if(try_scan_keyword('n'))
                        return {token{token_tag::keyword_fn, line_no_}};
                    return scan_name();
                case 'i':
                    if(try_scan_keyword('f'))
                        return {token{token_tag::keyword_if, line_no_}};
                    return scan_name();
                case 'l':
                    if(try_scan_keyword('e', 't'))
                        return {token{token_tag::keyword_let, line_no_}};
                    return scan_name();
                case 't':
                    if(try_scan_keyword('h', 'e', 'n'))
                        return {token{token_tag::keyword_then, line_no_}};
                    if(try_scan_keyword('y', 'p', 'e'))
                        return {token{token_tag::keyword_type, line_no_}};
                    return scan_name();
                case 'v':
                    if(try_scan_keyword('e', 'c', 't', 'o', 'r'))
                        return {token{token_tag::keyword_vector, line_no_}};
                    return scan_name();
                case '\0':
                    return {token{token_tag::stop, line_no_}};
                default:
                    return failed(error::invalid_character, line_no_, *p_);
            }
            // clang-format on
        }


        std::optional<token> skip_spaces_and_comments() noexcept {
            // clang-format off
            for(;;)
                switch(*p_) {
                    case ' ': case '\t': case '\r':
                        ++p_; continue;
                    case '\n':
                        ++p_; ++line_no_; continue;
                    case '-':
                        ++p_;
                        switch(*p_) {
                            case '-':
                                skip_comment();
                                continue;
                            case '>':
                                ++p_;
                                return {token{token_tag::minus_greater, line_no_}};
                            default:
                                return {token{token_tag::minus, line_no_}};
                        }
                    default:
                        return std::nullopt;
                }
            // clang-format on
        }


        void skip_comment() noexcept {
            ++p_;
            for(;;)
                switch(*p_) {
                case '\0':
                    return;
                case '\n':
                    ++p_;
                    return;
                default:
                    ++p_;
                    continue;
                }
        }


        tl::expected<token, error_info> scan_number() noexcept {
            // clang-format off
            char const* q = p_;
            ++q;
            bool has_point = false;
            bool has_exponent = false;
            for(;;)
                switch(*q) {
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                        ++q;
                        continue;
                    case '.':
                        if(has_point)
                            return failed(error::invalid_number, line_no_);
                        ++q;
                        switch(*q) {
                            case '0': case '1': case '2': case '3': case '4':
                            case '5': case '6': case '7': case '8': case '9':
                                has_point = true;
                                continue;
                            default:
                                return failed(error::invalid_number, line_no_);
                        }
                        continue;
                    case 'e': case 'E':
                        if(has_exponent)
                            return failed(error::invalid_number, line_no_);
                        has_exponent = true;
                        ++q;
                        switch(*q) {
                            case '+': case '-':
                                ++q;
                                continue;
                            case '0': case '1': case '2': case '3': case '4':
                            case '5': case '6': case '7': case '8': case '9':
                                continue;
                            default:
                                return failed(error::invalid_number, line_no_);
                        }
                        continue;
                    default:
                        std::swap(p_, q);
                        if(has_point || has_exponent)
                            return convert_floating_point(q, p_);
                        else
                            return convert_integer(q, p_);
                }
            // clang-format on
        }


        tl::expected<token, error_info> convert_floating_point(char const* from, char const* until) noexcept {
            double result;
            auto const converted = std::from_chars(from, until, result);
            if(converted.ec != std::errc{})
                return failed(error::invalid_number, line_no_);
            return {token{result, line_no_}};
        }


        tl::expected<token, error_info> convert_integer(char const* from, char const* until) noexcept {
            platform::integer result;
            auto const converted = std::from_chars(from, until, result);
            if(converted.ec != std::errc{})
                return failed(error::invalid_number, line_no_);
            return {token{result, line_no_}};
        }


        tl::expected<token, error_info> scan_name() noexcept {
            char const* q = p_;
            ++q;
            while(is_letter_or_digit(*q))
                ++q;
            std::swap(p_, q);
            return {token{std::string_view{q, std::size_t(p_ - q)}, line_no_}};
        }


        static bool is_letter_or_digit(char c) noexcept {
            // clang-format off
            switch(c) {
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
                case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
                case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': case 'a': case 'b': case 'c': case 'd':
                case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
                case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                case 'y': case 'z': case '_': case '0': case '1': case '2': case '3': case '4': case '5': case '6':
                case '7': case '8': case '9':
                    return true;
                default:
                    return false;
            }
            // clang-format on
        }


        bool try_scan_keyword(char c) noexcept {
            if(p_[1] != c)
                return false;
            if(is_letter_or_digit(p_[2]))
                return false;
            p_ += 2;
            return true;
        }


        bool try_scan_keyword(char c1, char c2) noexcept {
            if(p_[1] != c1 || p_[2] != c2)
                return false;
            if(is_letter_or_digit(p_[3]))
                return false;
            p_ += 3;
            return true;
        }


        bool try_scan_keyword(char c1, char c2, char c3) noexcept {
            if(p_[1] != c1 || p_[2] != c2 || p_[3] != c3)
                return false;
            if(is_letter_or_digit(p_[4]))
                return false;
            p_ += 4;
            return true;
        }


        bool try_scan_keyword(char c1, char c2, char c3, char c4, char c5) noexcept {
            if(p_[1] != c1 || p_[2] != c2 || p_[3] != c3 || p_[4] != c4 || p_[5] != c5)
                return false;
            if(is_letter_or_digit(p_[6]))
                return false;
            p_ += 6;
            return true;
        }


        bool try_scan_keyword(char c1, char c2, char c3, char c4, char c5, char c6) noexcept {
            if(p_[1] != c1 || p_[2] != c2 || p_[3] != c3 || p_[4] != c4 || p_[5] != c5 || p_[6] != c6)
                return false;
            if(is_letter_or_digit(p_[7]))
                return false;
            p_ += 7;
            return true;
        }

    }; // scanner

} // namespace mandalang
