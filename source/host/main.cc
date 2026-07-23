#include "cli/Argument.hh"
#include "cli/Parser.hh"
#include "decode/Decoder.hh"
#include "decode/DecoderResult.hh"
#include "elf/Parser.hh"

#include <print>

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

    auto decode = Decoder::decode(parse->revoFunctions);
    if (!decode) {
        Console::error("Failed to decode: {}", decode.error());
        return 1;
    }

    return 0;
}
