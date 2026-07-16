#include <print>

#include "decode/Decoder.hh"
#include "file/ELF.hh"

using namespace Revo;

int
main() {
#ifdef BUILD_DEBUG
    Console::set_level(Console::LogLevel::Debug);
#else
    Console::set_level(Console::LogLevel::Info);
#endif

    auto elf = ELF::parse("example.elf");
    if (!elf) {
        Console::error("Failed to parse ELF file: {}", elf.error());
        return 1;
    }
    Console::success("Successfully parsed ELF file");

    auto decoder = Decoder::decode(*elf);
    if (!decoder) {
        Console::error("Failed to decode: {}", decoder.error());
        return 1;
    }
    Console::success("Successfully decoded functions");

    return 0;
}
