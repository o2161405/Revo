#include "cfg/Builder.hh"
#include "decode/Decoder.hh"
#include "elf/Parser.hh"

#include <contracts>
#include <cstdlib>

using namespace Revo;

void
handle_contract_violation(const std::contracts::contract_violation& violation) {
    const auto location = violation.location();
    Console::error("Contract violation at {}:{} - {}", //
        location.file_name(), location.line(), violation.comment());
    std::abort();
}

int
main() {
    Console::set(Console::LogLevel::Debug);

    auto object = ELF::parse("input.elf");
    if (!object) {
        Console::error("Failed to parse ELF file: {}", object.error());
        return 1;
    }

    auto functions = Decode::decode(object->revo_functions);
    if (!functions) {
        Console::error("Failed to decode: {}", functions.error());
        return 1;
    }

    auto graph = CFG::build(*functions);
    if (!graph) {
        Console::error("Failed to build CFG: {}", graph.error());
        return 1;
    }

    return 0;
}
