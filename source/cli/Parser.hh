#pragma once

#include "cli/Argument.hh"
#include "cli/ArgumentSpecification.hh"
#include "cli/Concepts.hh"

#include <expected>
#include <filesystem>
#include <flat_map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>

namespace Revo::CLI {

class Parser {
public:
    struct Result {
        std::flat_map<Argument::Type, Argument::Variant> arguments;

        [[nodiscard]] bool
        contains(Argument::Type type) const {
            return arguments.contains(type);
        }

        template <Argument::Type TType>
            requires HasType<ArgumentSpecification<TType>>
        [[nodiscard]] std::optional<typename ArgumentSpecification<TType>::Type>
        get() const {
            if (const auto it = arguments.find(TType); it != arguments.end()) {
                return std::get<typename ArgumentSpecification<TType>::Type>(it->second);
            }

            return std::nullopt;
        }
    };

    [[nodiscard]] static std::expected<Parser::Result, std::string>
    parse(int argc, const char* const* argv);

    static void
    print_usage();

private:
    Parser() = default;

    // Parsing steps
    [[nodiscard]] static std::expected<void, std::string>
    parse_arguments(Parser::Result& result, std::span<const char* const> arguments);

    [[nodiscard]] static std::expected<void, std::string>
    check_required(const Parser::Result& result);

    // Utility functions
    template <typename TType>
    [[nodiscard]] static std::expected<TType, std::string>
    parse_value(std::string_view value);
};

} // namespace Revo::CLI
