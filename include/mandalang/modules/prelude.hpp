#pragma once


#include <memory>


#include <nonstd/memory_pool.hpp>
#include <tl/expected.hpp>

#include <mandalang/ir.hpp>
#include <mandalang/error_info.hpp>
#include <mandalang/scope.hpp>


namespace mandalang::modules {


    class prelude {
        scope exported_;
        nonstd::memory_pool<symbol> symbols_;
    public:

        prelude() noexcept = default;

        scope const& exported() noexcept {
            return exported_;
        }

        static tl::expected<std::unique_ptr<prelude>, error_info> initialize() {
            auto m = std::make_unique<prelude>();
            m->exported_.define(m->symbols_.create("integer", type{type_tag::integer}));
            m->exported_.define(m->symbols_.create("double", type{type_tag::floating_point}));;
            m->exported_.define(m->symbols_.create("boolean", type{type_tag::boolean}));
            m->exported_.define(m->symbols_.create("false", value{false}));
            m->exported_.define(m->symbols_.create("true", value{true}));
            return {std::move(m)};
        }
    };

} // namespace mandalang::modules
