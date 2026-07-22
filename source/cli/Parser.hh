#pragma once

#include "cli/Argument.hh"
#include "cli/ParserResult.hh"
#include "cli/Specification.hh"

#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace Revo::CLI {

class Parser {
public:
    [[nodiscard]] static std::expected<ParserResult, std::string>
    parse(int argc, const char* const* argv);

    static void
    print_usage();

private:
    Parser() = default;

    // --- Parsing steps ---
    [[nodiscard]] static std::expected<void, std::string>
    parse_arguments(ParserResult& result, std::span<const char* const> arguments);

    [[nodiscard]] static std::expected<void, std::string>
    check_required(const ParserResult& result);
};

} // namespace Revo::CLI
