#pragma once

#include "cli/Argument.hh"
#include "util/Console.hh"
#include "util/Types.hh"

#include <array>
#include <filesystem>
#include <string_view>

namespace Revo::CLI {

using namespace std::literals::string_view_literals;

template <Argument::Type TType>
struct ArgumentSpecification;

template <>
struct ArgumentSpecification<Argument::Type::Input> {
    using Type = std::filesystem::path;
    static constexpr auto arguments = std::array{"--input"sv, "-i"sv};
    static constexpr auto description = "Input file to be virtualized"sv;
    static constexpr bool required = true;
};

template <>
struct ArgumentSpecification<Argument::Type::Console> {
    using Type = Revo::Console::LogLevel;
    static constexpr auto arguments = std::array{"--console"sv};
    static constexpr auto description = "Granularity of output from virtualization steps"sv;
};

template <>
struct ArgumentSpecification<Argument::Type::Help> {
    static constexpr auto arguments = std::array{"--help"sv, "-h"sv};
};

} // namespace Revo::CLI
