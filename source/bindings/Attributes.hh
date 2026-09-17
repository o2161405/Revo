#pragma once

#include <algorithm>
#include <meta>
#include <string_view>

namespace Revo::Bindings {

struct ReprText {
    const char* text;

    consteval ReprText(std::string_view text) : text(std::define_static_string(text)) {}
};

struct ReprFormat {
    const char* format;

    consteval ReprFormat(std::string_view format) : format(std::define_static_string(format)) {}
};

} // namespace Revo::Bindings
