#include <print>

#include "decode/Decoder.hh"
#include "elf/Parser.hh"
#include "util/cli/Help.hh"
#include "util/cli/Options.hh"
#include "util/cli/OptionStorage.hh"
#include "util/cli/Parser.hh"
#include "util/cli/Shell.hh"

using namespace Revo;

int
main(int argc, char* argv[]) {
#ifdef BUILD_DEBUG
    Console::set(Console::LogLevel::Debug);
#else
    Console::set(Console::LogLevel::Info);
#endif

	using Storage = Util::OptionStorage<Util::Options>;
	auto cli = Util::Shell::from_args(argc, argv);
	auto result = Util::Parser<Storage>::parse(cli);
	if (not result) {
		Console::error("{}", result.error());
		return 1;
	}
	auto options = std::move(*result);
	// Idk just some example, ig we should make a task enum?
	auto task = options.template get<"task">();
	if (task == 1) {
		Console::success("Help Menu: ");
		Util::print_help<Util::Options>(cli);
		return 0;
	}

    auto elf = ELF::Parser::parse(options.template get<"input">());
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
