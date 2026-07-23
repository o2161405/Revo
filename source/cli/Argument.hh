#pragma once

#include "util/Console.hh"
#include "util/Types.hh"

#include <filesystem>
#include <variant>

namespace Revo::CLI {

struct Argument {
    enum class Type : u8 {
        Input,
        Console,
        Help,
    };

    /* clang-format off */
    using Variant = std::variant<
        std::monostate,
        std::filesystem::path,
        Console::LogLevel
    >;
    /* clang-format on */

    Type type;
    Variant value;
};

} // namespace Revo::CLI
