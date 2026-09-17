#pragma once

#include <meta>
#include <string_view>

namespace Revo::Bindings {

struct Annotation {
    struct Flag {};

    struct Arithmetic {};
};

struct Repr {
    struct Text {
        const char* text;

        consteval Text(std::string_view text) : text{std::define_static_string(text)} {}
    };

    struct Format {
        const char* format;

        consteval Format(std::string_view format) : format{std::define_static_string(format)} {}
    };
};

} // namespace Revo::Bindings
