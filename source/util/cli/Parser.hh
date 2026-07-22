#pragma once

#include "OptionStorage.hh"
#include "Shell.hh"
#include "ValueParser.hh"
#include "../Util.hh"

#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <format>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Revo::Util {
	template<typename TOptionStorage>
	struct Parser;

	template<typename... TOpts>
	struct Parser<OptionStorage<std::tuple<TOpts...>>> {
		using OptionsType = std::tuple<TOpts...>;
		using OptionStorageType = OptionStorage<OptionsType>;

	private:
		static constexpr std::size_t OptionCount = sizeof...(TOpts);
		static constexpr std::size_t PositionalCount = (std::size_t{0} + ... + (TOpts::IsPositional ? std::size_t{1} : std::size_t{0}));

		template<std::size_t VIndex>
		using OptionType = std::tuple_element_t<VIndex, OptionsType>;

		template<typename>
		static constexpr bool AlwaysFalse = false;

		struct MatchResult {
			std::size_t Index = OptionCount;
			std::size_t Count = 0;
		};

		static_assert(
			((not TOpts::IsPositional or not IsVector<typename TOpts::ValueType>) and ...),
			"Vector positional arguments are not currently supported"
		);

		template<typename TOpt>
		static consteval bool has_value() {
			return not std::is_void_v<typename TOpt::ValueType> and not TOpt::storage_key().empty();
		}

		template<typename TOpt>
		static consteval bool has_callback() {
			using CallbackType = std::remove_cvref_t<decltype(TOpt::callback())>;
			return not std::same_as<CallbackType, std::nullptr_t>;
		}

		template<std::size_t... VIndices>
		static consteval auto make_positional_indices(std::index_sequence<VIndices...>) {
			std::array<std::size_t, PositionalCount> result{};
			std::size_t output_index = 0;

			([&] {
				using OptType = OptionType<VIndices>;

				if constexpr (OptType::IsPositional) {
					result[output_index++] = VIndices;
				}
			}(), ...);

			return result;
		}

		static constexpr auto PositionalIndices = make_positional_indices(std::make_index_sequence<OptionCount>{});

		template<typename TOpt>
		static constexpr bool matches_named(std::string_view argument) {
			if constexpr (TOpt::IsPositional) {
				return false;
			}

			if (argument.starts_with("--")) {
				return argument.size() > 2 and argument.substr(2) == TOpt::name();
			}

			if (argument.starts_with("-")) {
				return argument.size() == 2 and TOpt::short_name() != '\0' and argument[1] == TOpt::short_name();
			}

			return false;
		}

		template<std::size_t... VIndices>
		static constexpr MatchResult match_named_impl(std::string_view argument, std::index_sequence<VIndices...>) {
			MatchResult result{};

			([&] {
				using OptType = OptionType<VIndices>;

				if (matches_named<OptType>(argument)) {
					result.Index = VIndices;
					++result.Count;
				}
			}(), ...);

			return result;
		}

		static constexpr MatchResult match_named(std::string_view argument) {
			return match_named_impl(argument, std::make_index_sequence<OptionCount>{});
		}

		static constexpr bool is_named_option(std::string_view argument) {
			if (argument == "--") {
				return true;
			}

			const std::size_t separator = argument.find('=');
			const std::string_view option_text = separator == std::string_view::npos ? argument : argument.substr(0, separator);

			return match_named(option_text).Count != 0;
		}

		template<std::size_t VIndex = 0, typename TFn>
		static std::expected<void, std::string> dispatch(std::size_t index, TFn&& fn) {
			if constexpr (VIndex == OptionCount) {
				return std::unexpected("internal parser error: invalid option index");
			} else {
				if (index == VIndex) {
					return std::forward<TFn>(fn).template operator()<OptionType<VIndex>>();
				}

				return dispatch<VIndex + 1>(index, std::forward<TFn>(fn));
			}
		}

		template<typename TResult>
		static std::expected<void, std::string> normalize_callback_result(TResult&& result) {
			using ResultType = std::remove_cvref_t<TResult>;

			if constexpr (std::same_as<ResultType, bool>) {
				if (not result) {
					return std::unexpected("option callback rejected the argument");
				}

				return {};
			} else if constexpr (std::same_as<ResultType, std::expected<void, std::string>>) {
				return std::forward<TResult>(result);
			} else if constexpr (std::same_as<ResultType, std::expected<bool, std::string>>) {
				if (not result) {
					return std::unexpected(result.error());
				}

				if (not *result) {
					return std::unexpected("option callback rejected the argument");
				}

				return {};
			} else {
				static_assert(
					AlwaysFalse<ResultType>,
					"Option callback must return void, bool, std::expected<void, std::string>, or std::expected<bool, std::string>"
				);
			}
		}

		template<typename TOpt>
		static std::expected<void, std::string> invoke_callback(OptionStorageType& options, std::string_view argument) {
			if constexpr (not has_callback<TOpt>()) {
				return {};
			} else {
				constexpr auto callback = TOpt::callback();

				auto invoke = [&]() -> decltype(auto) {
					if constexpr (std::invocable<decltype(callback), OptionStorageType&, std::string_view>) {
						return std::invoke(callback, options, argument);
					} else if constexpr (std::invocable<decltype(callback), OptionStorageType&>) {
						return std::invoke(callback, options);
					} else if constexpr (std::invocable<decltype(callback), std::string_view>) {
						return std::invoke(callback, argument);
					} else if constexpr (std::invocable<decltype(callback)>) {
						return std::invoke(callback);
					} else {
						static_assert(AlwaysFalse<TOpt>, "Option callback must accept (), (std::string_view), (OptionStorageType&), or (OptionStorageType&, std::string_view)");
					}
				};

				using ResultType = decltype(invoke());

				if constexpr (std::is_void_v<ResultType>) {
					invoke();
					return {};
				} else {
					return normalize_callback_result(invoke());
				}
			}
		}

		template<typename TOpt>
		static std::expected<void, std::string> parse_value(OptionStorageType& options, std::string_view text) {
			using ValueType = typename TOpt::ValueType;

			static_assert(has_value<TOpt>(), "Option does not define a stored value");
			static_assert(OptionStorageType::template has<TOpt>(), "Option storage does not contain the option's key");

			if constexpr (IsVector<ValueType>) {
				auto result = ValueParser<ValueType>::parse(text);

				if (not result) {
					return std::unexpected(result.error());
				}

				options.template get<TOpt>().push_back(std::move(*result));
			} else {
				auto result = ValueParser<ValueType>::parse(text);

				if (not result) {
					return std::unexpected(result.error());
				}

				options.template set<TOpt>(std::move(*result));
			}

			return {};
		}

		template<typename TOpt>
		static std::expected<void, std::string> handle_positional(OptionStorageType& options, std::string_view argument) {
			if constexpr (has_value<TOpt>()) {
				auto result = parse_value<TOpt>(options, argument);

				if (not result) {
					return std::unexpected(std::format("invalid value for positional argument '{}': {}", TOpt::name(), result.error()));
				}
			}

			auto callback_result = invoke_callback<TOpt>(options, argument);

			if (not callback_result) {
				return std::unexpected(std::format("callback for positional argument '{}' failed: {}", TOpt::name(), callback_result.error()));
			}

			return {};
		}

		template<typename TOpt>
		static std::expected<void, std::string> handle_named(
			OptionStorageType& options,
			std::span<const std::string> arguments,
			std::size_t& argument_index,
			std::string_view argument
		) {
			const std::size_t separator = argument.find('=');
			const std::string_view option_text = separator == std::string_view::npos ? argument : argument.substr(0, separator);

			if constexpr (has_value<TOpt>()) {
				using ValueType = typename TOpt::ValueType;

				if constexpr (IsVector<ValueType>) {
					bool parsed_any = false;

					if (separator != std::string_view::npos) {
						const std::string_view value_text = argument.substr(separator + 1);
						auto result = parse_value<TOpt>(options, value_text);

						if (not result) {
							return std::unexpected(std::format("invalid value for '{}': {}", option_text, result.error()));
						}

						parsed_any = true;
					} else {
						while (argument_index + 1 < arguments.size()) {
							const std::string_view value_text = arguments[argument_index + 1];

							if (is_named_option(value_text)) {
								break;
							}

							auto result = parse_value<TOpt>(options, value_text);

							if (not result) {
								if (not parsed_any) {
									return std::unexpected(std::format("invalid value for '{}': {}", option_text, result.error()));
								}

								break;
							}

							++argument_index;
							parsed_any = true;
						}
					}

					if (not parsed_any) {
						return std::unexpected(std::format("argument '{}' expects at least one value", option_text));
					}
				} else {
					std::string_view value_text;

					if (separator != std::string_view::npos) {
						value_text = argument.substr(separator + 1);
					} else {
						++argument_index;

						if (argument_index >= arguments.size()) {
							return std::unexpected(std::format("argument '{}' expects a value", option_text));
						}

						value_text = arguments[argument_index];
					}

					auto result = parse_value<TOpt>(options, value_text);

					if (not result) {
						return std::unexpected(std::format("invalid value for '{}': {}", option_text, result.error()));
					}
				}
			} else if (separator != std::string_view::npos) {
				return std::unexpected(std::format("argument '{}' does not accept a value", option_text));
			}

			auto callback_result = invoke_callback<TOpt>(options, option_text);

			if (not callback_result) {
				return std::unexpected(std::format("callback for '{}' failed: {}", option_text, callback_result.error()));
			}

			return {};
		}

	public:
		[[nodiscard]]
		static std::expected<OptionStorageType, std::string> parse(const Shell& shell) {
			OptionStorageType options{};

			const auto arguments = shell.user_args();

			std::size_t positional_index = 0;
			bool named_options_enabled = true;

			for (std::size_t argument_index = 0; argument_index < arguments.size(); ++argument_index) {
				const std::string_view argument = arguments[argument_index];

				if (named_options_enabled and argument == "--") {
					named_options_enabled = false;
					continue;
				}

				const bool is_named = named_options_enabled and argument.size() > 1 and argument.starts_with("-");

				if (is_named) {
					const std::size_t separator = argument.find('=');
					const std::string_view option_text = separator == std::string_view::npos ? argument : argument.substr(0, separator);
					const MatchResult matches = match_named(option_text);

					if (matches.Count == 0) {
						return std::unexpected(std::format("unknown argument: '{}'", argument));
					}

					if (matches.Count != 1) {
						return std::unexpected(std::format("argument '{}' matches more than one option", option_text));
					}

					auto result = dispatch(matches.Index, [&]<typename TOpt>() -> std::expected<void, std::string> {
						return handle_named<TOpt>(options, arguments, argument_index, argument);
					});

					if (not result) {
						return std::unexpected(result.error());
					}
				} else {
					if (positional_index >= PositionalIndices.size()) {
						return std::unexpected(std::format("unexpected positional argument: '{}'", argument));
					}

					const std::size_t option_index = PositionalIndices[positional_index++];

					auto result = dispatch(option_index, [&]<typename TOpt>() -> std::expected<void, std::string> {
						return handle_positional<TOpt>(options, argument);
					});

					if (not result) {
						return std::unexpected(result.error());
					}
				}
			}

			return options;
		}

		[[nodiscard]]
		static std::expected<OptionStorageType, std::string> parse(int argc, const char* const* argv) {
			return parse(Shell::from_args(argc, argv));
		}
	};
}