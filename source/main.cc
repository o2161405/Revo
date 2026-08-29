#include "cfg/Builder.hh"
#include "cli/Argument.hh"
#include "cli/Parser.hh"
#include "decode/Decoder.hh"
#include "elf/Parser.hh"

#include <contracts>
#include <cstdlib>

void
handle_contract_violation(const std::contracts::contract_violation& violation) {
    const auto location = violation.location();
    Revo::Console::error("Contract violation at {}:{} - {}", //
        location.file_name(), location.line(), violation.comment());
    std::abort();
}

// definitely a good todo somewhat soon:
// put Decoder in Revo::Decode, and then static functions like Decoder::decode
// and Parser::parse should be free functions so it's called like ELF::parse
// instead of ELF::Parser::parse. also get rid of Decoder::Result

using namespace Revo;

int
main(int argc, const char* const* argv) {
    auto cli = CLI::Parser::parse(argc, argv);
    if (!cli) {
        Console::error("{}", cli.error());
        CLI::Parser::print_usage();
        return 1;
    }

    if (cli->contains(CLI::Argument::Type::Help)) {
        CLI::Parser::print_usage();
        return 0;
    }

    auto log_level = cli->get<CLI::Argument::Type::Console>();
    if (log_level) {
        Console::set(*log_level);
    }
    else {
        Console::set(Console::LogLevel::Info);
    }

    auto input_file = cli->get<CLI::Argument::Type::Input>();

    auto parse = ELF::Parser::parse(*input_file);
    if (!parse) {
        Console::error("Failed to parse ELF file: {}", parse.error());
        return 1;
    }

    auto decode = Decoder::decode(parse->revo_functions);
    if (!decode) {
        Console::error("Failed to decode: {}", decode.error());
        return 1;
    }

    auto graph = CFG::Builder::build(decode->functions);
    if (!graph) {
        Console::error("Failed to build CFG: {}", graph.error());
        return 1;
    }

    return 0;
}
