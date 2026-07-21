#include <print>

#include "decode/Decoder.hh"
#include "decode/DecoderResult.hh"
#include "elf/Parser.hh"
#include "elf/ParserResult.hh"

using namespace Revo;

int
main() {
#ifdef BUILD_DEBUG
    Console::set(Console::LogLevel::Debug);
#else
    Console::set(Console::LogLevel::Info);
#endif

    auto parse_result = ELF::Parser::parse("example.elf");
    if (!parse_result) {
        Console::error("Failed to parse ELF file: {}", parse_result.error());
        return 1;
    }
    Console::success("Successfully parsed ELF file");

    auto decode_result = Decoder::decode(parse_result->revoFunctions);
    if (!decode_result) {
        Console::error("Failed to decode: {}", decode_result.error());
        return 1;
    }
    Console::success("Successfully decoded functions");

    return 0;
}
