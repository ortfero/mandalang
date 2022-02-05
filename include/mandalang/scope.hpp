#pragma once


#include <unordered_map>

#include <nonstd/memory_pool.hpp>
#include <tl/expected.hpp>

#include <mandalang/ir.hpp>
#include <mandalang/error_info.hpp>
#include <mandalang/function.hpp>


namespace mandalang {


    class scope {
        scope* outer_;
        std::unordered_map<std::string_view, symbol*> symbols_;

    public:

        explicit scope(scope* outer = nullptr) noexcept: outer_{outer} { }


        tl::expected<symbol const*, error_info> define(symbol* symbol) {
            auto const name = symbol->name;
            auto const result = symbols_.try_emplace(name, symbol);
            if(!result.second)
                return failed(error::duplicated_name, 0, result.first->first);
            return result.first->second;
        }


        symbol const* redefine(std::string_view name, value const& value, nonstd::memory_pool<symbol>& symbols) {
            auto const found = symbols_.find(name);
            if(found == symbols_.end()) {
                auto* created = symbols.create(name, value);
                symbols_.try_emplace(name, created);
                return created;
            } else {
                found->second->tag = symbol_tag::value;
                found->second->value = value;
                return found->second;
            }
        }


        symbol const* redefine(std::string_view name, type const& type, nonstd::memory_pool<symbol>& symbols) {
            auto const found = symbols_.find(name);
            if(found == symbols_.end()) {
                auto* created = symbols.create(name, type);
                symbols_.try_emplace(name, created);
                return created;
            } else {
                found->second->tag = symbol_tag::type;
                found->second->type = type;
                return found->second;
            }
        }


        symbol const* find(std::string_view name) const noexcept {
            auto const found = symbols_.find(name);
            if(found == symbols_.end())
                return outer_ ? outer_->find(name) : nullptr;
            return found->second;
        }

        symbol* find_local(std::string_view name) noexcept {
            auto const found = symbols_.find(name);
            if(found == symbols_.end())
                return nullptr;
            return found->second;
        }



        tl::expected<void, error_info> import(scope const& other, std::vector<std::string_view> const& names) noexcept {
            for(auto const& name: names) {
                auto const found = other.symbols_.find(name);
                if(found == other.symbols_.end())
                    return failed(error::name_is_not_found_to_import, name);
                auto const defined = define(found->second);
                if(!defined)
                    return tl::make_unexpected(defined.error());
            }
            return {};
        }


        tl::expected<void, error_info> import(scope const& other) noexcept {
            for(auto const& [name, symbol_ptr]: other.symbols_) {
                auto const defined = define(symbol_ptr);
                if(!defined)
                    return tl::make_unexpected(defined.error());
            }
            return {};
        }

    }; // scope


} // namespace mandalang
