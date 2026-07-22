#pragma once

#include "Option.hh"

#include <tuple>

namespace Revo::Util {
	using InputOption = decltype(Option
		.name<"input">()
		.description<"Input file">()
		.storage_info<"input", std::filesystem::path>()
		.positional()
	);

	// I usually have a "task" value in my option storage to know which action the program should take
	using TaskOption = decltype(Option
		.name<"task">()
		.storage_info<"task", int>()
		.hidden()
	);

	using HelpOption = decltype(Option
		.name<"help">()
		.description<"Displays the help menu">()
		.short_name<'h'>()
		.storage_info<"task", void>()
		.callback<[](auto& opts) {
			opts.template set<"task">(1);
		}>()
	);
	
	// A `std::tuple` for the option types, no type map helper here
	// I could provide one if its worth it
	using Options = std::tuple<
		InputOption,
		TaskOption,
		HelpOption
	>;
}