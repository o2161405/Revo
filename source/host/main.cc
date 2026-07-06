#include <print>

#include "file/ELF.hh"

using namespace Revo;

int
main() {
#ifdef BUILD_DEBUG
    Console::set_level(Console::LogLevel::Debug);
#else
    Console::set_level(Console::LogLevel::Info);
#endif

    auto elf = File::ELF::parse("example.elf");
    if (!elf) {
        Console::error("Failed to parse ELF file: {}", elf.error());
        return 1;
    }

    Console::success("Successfully parsed ELF file");
    return 0;
}
