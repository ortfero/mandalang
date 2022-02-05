#pragma once


#include <memory>
#include <string>

#include <tl/expected.hpp>

#include <mandalang/ir.hpp>
#include <mandalang/error_info.hpp>
#include <mandalang/mod.hpp>
#include <mandalang/modules/prelude.hpp>
#include <mandalang/type.hpp>


namespace mandalang {

    class engine {

        mod default_module_;
        std::unique_ptr<modules::prelude> prelude_;

        engine() noexcept = default;

    public:

        static tl::expected<std::unique_ptr<engine>, error_info> create() noexcept {
            try {
                auto engine = std::unique_ptr<class engine>{new class engine()};
                auto expected_prelude = modules::prelude::initialize();
                if(!expected_prelude)
                    return tl::make_unexpected(expected_prelude.error());
                engine->prelude_ = std::move(*expected_prelude);
                engine->default_module_.import(engine->prelude_->exported());
                return {std::move(engine)};
            } catch (std::bad_alloc const&) {
                return failed(error::not_enough_memory);
            }
        }


        tl::expected<value, error_info> evaluate_expression(std::string const& source) noexcept {
            try {
                return default_module_.evaluate_expression(std::move(source));
            } catch (std::bad_alloc const&) {
                return failed(error::not_enough_memory);
            }
        }


        tl::expected<symbol_or_value, error_info> evaluate_definition_or_expression(std::string source) noexcept {
            try {
                return default_module_.evaluate_definition_or_expression(std::move(source));
            } catch (std::bad_alloc const&) {
                return failed(error::not_enough_memory);
            }
        }


        tl::expected<symbol const*, error_info> redefine(std::string_view name, value const& value) {
            try {
                return {default_module_.redefine(name, value)};
            } catch (std::bad_alloc const&) {
                return failed(error::not_enough_memory);
            }
        }


    }; // engine

} // namespace mandalang