#pragma once

#include "Shell.hh"
#include "ValueParser.hh"

#include <cstddef>
#include <format>
#include <print>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Revo::Util {
	namespace Detail {
		template<typename TOptions, typename TFunction, std::size_t... VIndices>
		void for_each_option_impl(TFunction&& function, std::index_sequence<VIndices...>) {
			(function.template operator()<std::tuple_element_t<VIndices, TOptions>>(), ...);
		}

		template<typename TOptions, typename TFunction>
		void for_each_option(TFunction&& function) {
			for_each_option_impl<TOptions>(
				std::forward<TFunction>(function),
				std::make_index_sequence<std::tuple_size_v<TOptions>>{}
			);
		}

		template<typename TOpt>
		inline constexpr bool OptionHasValue =
			not std::is_void_v<typename TOpt::ValueType>
			and not TOpt::storage_key().empty();

		template<typename TOptions>
		inline constexpr std::size_t PositionalCount = []<std::size_t... VIndices>(std::index_sequence<VIndices...>) consteval {
			return (std::size_t{0} + ... + (std::tuple_element_t<VIndices, TOptions>::IsPositional ? std::size_t{1} : std::size_t{0}));
		}(std::make_index_sequence<std::tuple_size_v<TOptions>>{});
	}

	template<typename TOptions>
	void print_help(const Shell& shell) {
		static constexpr std::size_t OptionCount = std::tuple_size_v<TOptions>;
		static constexpr std::size_t PositionalCount = Detail::PositionalCount<TOptions>;
		static constexpr std::size_t NamedCount = OptionCount - PositionalCount;

		const std::string_view program_name = shell.program_name().empty()
			? std::string_view{"program"}
			: shell.program_name();

		std::string usage = std::format("\t{}", program_name);

		Detail::for_each_option<TOptions>([&]<typename TOpt> {
			if constexpr (TOpt::IsPositional) {
				usage += std::format(" [{}]", TOpt::name());
			}
		});

		if constexpr (NamedCount > 0) {
			usage += " [options]";
		}

		std::println("Usage:");
		std::println("{}", usage);

		if constexpr (PositionalCount > 0) {
			std::println("\nPositional arguments:");

			Detail::for_each_option<TOptions>([]<typename TOpt> {
				if constexpr (TOpt::IsPositional) {
					std::string option_text{TOpt::name()};

					if constexpr (Detail::OptionHasValue<TOpt>) {
						using ParserType = ValueParser<typename TOpt::ValueType>;
						option_text += std::format(" <{}>", ParserType::HelpHint);
					}

					std::println("\t{:<24} {}", option_text, TOpt::description());
				}
			});
		}

		if constexpr (NamedCount > 0) {
			std::println("\nOptions:");

			Detail::for_each_option<TOptions>([]<typename TOpt> {
				if constexpr (not TOpt::IsPositional) {
					std::string option_text;

					if constexpr (TOpt::short_name() != '\0') {
						option_text += std::format("-{}, ", TOpt::short_name());
					}

					option_text += std::format("--{}", TOpt::name());

					if constexpr (Detail::OptionHasValue<TOpt>) {
						using ParserType = ValueParser<typename TOpt::ValueType>;
						option_text += std::format("=<{}>", ParserType::HelpHint);
					}

					std::println("\t{:<24} {}", option_text, TOpt::description());
				}
			});
		}
	}
}