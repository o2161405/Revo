#include <print>

#include "decode/Decoder.hh"
#include "elf/Parser.hh"

using namespace Revo;

int
main() {
#ifdef BUILD_DEBUG
    Console::set(Console::LogLevel::Debug);
#else
    Console::set(Console::LogLevel::Info);
#endif

    auto elf = ELF::Parser::parse("example.elf");
    if (!elf) {
        Console::error("Failed to parse ELF file: {}", elf.error());
        return 1;
    }
    Console::success("Successfully parsed ELF file");

    auto decoder = Decoder::decode(elf->functions());
    if (!decoder) {
        Console::error("Failed to decode: {}", decoder.error());
        return 1;
    }
    Console::success("Successfully decoded functions");

    return 0;
}
