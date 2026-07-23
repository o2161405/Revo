#include "Parser.hh"

#include "util/Console.hh"

#include <algorithm>
#include <cctype>
#include <format>
#include <meta>
#include <ranges>

namespace Revo::CLI {

namespace {

template <typename TType>
[[nodiscard]] static std::expected<TType, std::string>
parse_value(std::string_view value);

template <>
std::expected<std::filesystem::path, std::string>
parse_value(std::string_view value) {
    return std::filesystem::path{value};
}

template <>
std::expected<Console::LogLevel, std::string>
parse_value(std::string_view value) {
    /* clang-format off */
    constexpr auto equal_string = [](std::string_view value, std::string_view identifier) -> bool {
        return std::ranges::equal(value, identifier, [](unsigned char a, unsigned char b) { //
            return std::tolower(a) == std::tolower(b);
        });
    };
    /* clang-format on */

    template for (constexpr auto enumerator :
        std::define_static_array(std::meta::enumerators_of(^^Console::LogLevel))) {
        if (equal_string(value, std::meta::identifier_of(enumerator))) {
            return [:enumerator:];
        }
    }

    return std::unexpected(std::format("Invalid log level \"{}\"", value));
}

template <typename TEnum>
[[nodiscard]] consteval std::string_view
enum_string() {
    /* clang-format off */
    return std::define_static_string(std::meta::enumerators_of(^^TEnum) //
        | std::views::transform([](std::meta::info enumerator) { return std::meta::identifier_of(enumerator); }) //
        | std::views::join_with("|"sv) //
        | std::ranges::to<std::string>());
    /* clang-format on */
}

} // namespace

std::expected<ParserResult, std::string>
Parser::parse(int argc, const char* const* argv) {
    ParserResult result;

    const auto argument_count = argc > 1 ? static_cast<std::size_t>(argc - 1) : 0uz;
    const std::span<const char* const> arguments{argv + 1, argument_count};

    return parse_arguments(result, arguments)
        .and_then([&]() { return check_required(result); })
        .transform([&]() { return std::move(result); });
}

std::expected<void, std::string>
Parser::parse_arguments(ParserResult& result, std::span<const char* const> arguments) {
    for (auto i{0uz}; i < arguments.size(); ++i) {
        const std::string_view flag{arguments[i]};
        bool valid{false};

        template for (constexpr auto enumerator :
            std::define_static_array(std::meta::enumerators_of(^^Argument::Type))) {
            constexpr auto argument_type = [:enumerator:];
            using Specification = ArgumentSpecification<argument_type>;

            if (valid || !std::ranges::contains(Specification::arguments, flag)) {
                continue;
            }

            valid = true;

            if (result.contains(argument_type)) {
                return std::unexpected(std::format("Argument {} has already been specified", flag));
            }

            if constexpr (HasType<Specification>) {
                if (i + 1 >= arguments.size()) {
                    return std::unexpected(
                        std::format("Argument {} requires a value", flag.substr(2)));
                }

                auto value = parse_value<typename Specification::Type>(arguments[++i]);
                if (!value) {
                    return std::unexpected(std::format("{}", value.error()));
                }

                result.arguments.emplace(argument_type, std::move(*value));
            }
            else {
                result.arguments.emplace(argument_type, std::monostate{});
            }
        }

        if (!valid) {
            return std::unexpected(std::format("Invalid argument {}", flag));
        }
    }

    return {};
}

std::expected<void, std::string>
Parser::check_required(const ParserResult& result) {
    if (result.contains(Argument::Type::Help)) {
        return {};
    }

    template for (constexpr auto enumerator :
        std::define_static_array(std::meta::enumerators_of(^^Argument::Type))) {
        constexpr auto argument_type = [:enumerator:];
        using Specification = ArgumentSpecification<argument_type>;

        if constexpr (HasRequired<Specification>) {
            if constexpr (Specification::required) {
                if (!result.contains(argument_type)) {
                    return std::unexpected(
                        std::format("Missing required argument {}", Specification::arguments[0]));
                }
            }
        }
    }

    return {};
}

void
Parser::print_usage() {
    Console::none("\nArguments:");

    template for (constexpr auto enumerator :
        std::define_static_array(std::meta::enumerators_of(^^Argument::Type))) {
        constexpr auto argument_type = [:enumerator:];
        using Specification = ArgumentSpecification<argument_type>;

        std::string_view description = "";
        if constexpr (HasDescription<Specification>) {
            description = Specification::description;
        }

        Console::none("\t{:<15n:s} {}", Specification::arguments, description);

        if constexpr (HasType<Specification>) {
            if constexpr (std::is_enum_v<typename Specification::Type>) {
                Console::none("\t{:<15} Available choices: {}", "",
                    enum_string<typename Specification::Type>());
            }
        }
    }
}

} // namespace Revo::CLI
