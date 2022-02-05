#pragma once


#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <system_error>

#include <mandalang/error_info.hpp>
#include <mandalang/mod.hpp>


namespace mandalang {

    class loader {
    public:

        std::unique_ptr<mod> load(std::string const& path, std::system_error& se) {
            auto maybe_source = read(path, se);
            if(!maybe_source)
                return nullptr;
            auto m = std::make_unique<mod>(std::move(*source));

            return m;
        }

    private:


        std::optional<std::string> read(std::string const& path, std::system_error& se) noexcept {
            using namespace std;
            unique_ptr<FILE, int (*)(FILE *)>
                    file{fopen(path.data(), "rb"), fclose};
            if (!file)
                return failed(nullopt, se, static_cast<std::errc>(errno));
            fseek(file.get(), 0, SEEK_END);
            auto const file_size = std::size_t(ftell(file.get()));
            if (file_size == std::size_t(-1L))
                return failed(nullopt, se, static_cast<std::errc>(errno));
            fseek(file.get(), 0, SEEK_SET);
            try {
                string source(file_size, '\0');
                auto const bytes_read = fread(source.data(), 1, file_size, file.get());
                if (bytes_read != file_size)
                    failed(nullopt, se, static_cast<std::errc>(errno));
                return std::move(source);
            } catch (std::bad_alloc const& ba) {
                return failed(nullopt, se, error::not_enough_memory);
            }
        }

    }; // loader

} // namespace mandalang
