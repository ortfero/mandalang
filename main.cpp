#include <cstdio>
#include <exception>

#include <ufmt/print.hpp>

#include <mandalang/engine.hpp>


constexpr auto input_buffer_size = 1023u;
char input_buffer[input_buffer_size + 1];


int main() {
    try {
        auto expected_engine = mandalang::engine::create();
        if(!expected_engine) {
            ufmt::error("[error] ", mandalang::format_without_line(expected_engine.error()));
            return EXIT_FAILURE;
        }
        auto engine = std::move(*expected_engine);
        ufmt::print("Mandalang interactive session");
        std::system_error se{std::error_code{}};
        for(;;) {
            std::fgets(input_buffer, input_buffer_size, stdin);
            input_buffer[input_buffer_size] = '\0';
            if (input_buffer[0] == '\n')
                return EXIT_SUCCESS;
            auto definition_or_value = engine->evaluate_definition_or_expression(std::string{input_buffer});
            if(!definition_or_value) {
                ufmt::error("[error] ", mandalang::format_without_line(definition_or_value.error()));
                continue;
            }
            if(definition_or_value->tag == mandalang::symbol_or_value_tag::symbol) {
                ufmt::print(*definition_or_value->symbol);
                continue;
            }
            auto expected_redefined = engine->redefine(std::string_view{"_"}, definition_or_value->value);
            if(!expected_redefined) {
                ufmt::error("[error] ", mandalang::format_without_line(expected_redefined.error()));
            }
            ufmt::print(**expected_redefined);
        }
    } catch (std::exception const& e) {
        ufmt::error("[error] ", std::string_view{e.what()});
        return EXIT_FAILURE;
    } catch(...) {
        ufmt::error("[error] Unknown exception");
        return EXIT_FAILURE;
    }
}
