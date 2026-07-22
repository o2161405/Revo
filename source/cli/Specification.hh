#pragma once

#include "cli/Argument.hh"
#include "util/Console.hh"
#include "util/Types.hh"

#include <array>
#include <filesystem>
#include <functional>
#include <string_view>

namespace Revo::CLI {

using namespace std::literals::string_view_literals;

template <typename TSpecification>
concept HasArguments = requires { TSpecification::arguments; };

template <typename TSpecification>
concept HasType = requires { typename TSpecification::Type; };

template <typename TSpecification>
concept HasRequired = requires { TSpecification::required; };

template <typename TSpecification>
concept HasDescription = requires { TSpecification::description; };

template <Argument::Type TType>
struct ArgumentSpecification;

template <>
struct ArgumentSpecification<Argument::Type::Input> {
    using Type = std::filesystem::path;
    static constexpr auto arguments = std::array{"--input"sv, "-i"sv};
    static constexpr std::string_view description = "Input file to be virtualized";
    static constexpr bool required = true;
};

template <>
struct ArgumentSpecification<Argument::Type::Console> {
    using Type = Revo::Console::LogLevel;
    static constexpr auto arguments = std::array{"--console"sv};
    static constexpr std::string_view description =
        "Granularity of output from virtualization steps";
};

template <>
struct ArgumentSpecification<Argument::Type::Help> {
    static constexpr auto arguments = std::array{"--help"sv, "-h"sv};
};

} // namespace Revo::CLI
