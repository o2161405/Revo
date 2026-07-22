#pragma once

#include <array>
#include <charconv>
#include <expected>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace Revo::Util {
	// Converts one option argument string into a typed value.
	template<typename TValue>
	struct ValueParser {
		// Hint displayed for the value in the help menu.
		static constexpr std::string_view HelpHint = "value";
	};

	template<>
	struct ValueParser<std::string> {
		static constexpr std::string_view HelpHint = "string";

		[[nodiscard]]
		static std::expected<std::string, std::string>
		parse(std::string_view text) {
			return std::string{text};
		}
	};

	template<>
	struct ValueParser<int> {
		static constexpr std::string_view HelpHint = "integer";

		[[nodiscard]]
		static std::expected<int, std::string>
		parse(std::string_view text) {
			if (text.empty()) {
				return std::unexpected("invalid integer value (empty)");
			}

			int value{};

			const char* const begin = text.data();
			const char* const end = text.data() + text.size();

			const auto result = std::from_chars(begin, end, value);

			if (result.ec != std::errc{} or result.ptr != end) {
				return std::unexpected(std::format("invalid integer value '{}'", text));
			}

			return value;
		}
	};

	template<>
	struct ValueParser<std::filesystem::path> {
		static constexpr std::string_view HelpHint = "path";

		[[nodiscard]]
		static std::expected<std::filesystem::path, std::string>
		parse(std::string_view text) {
			std::filesystem::path path{ std::string{text} };

			if (path.is_absolute()) {
				return path;
			}

			std::error_code error;
			auto currentPath = std::filesystem::current_path(error);

			if (error) {
				return std::unexpected(std::format("failed to resolve current directory: {}", error.message()));
			}

			return currentPath / path;
		}
	};

	template<typename TValue>
	struct ValueParser<std::vector<TValue>> {
		using ItemParser = ValueParser<TValue>;

	private:
		// Constructs "<item hint>s..." entirely at compile time.
		// some bullsh*t because idfk why format wont work for me
		static consteval auto make_help_hint() {
			constexpr std::string_view ItemHelpHint = ItemParser::HelpHint;

			std::array<char, ItemHelpHint.size() + 4> result{};

			std::size_t index = 0;

			for (char character : ItemHelpHint) {
				result[index++] = character;
			}

			result[index++] = 's';
			result[index++] = '.';
			result[index++] = '.';
			result[index++] = '.';

			return result;
		}

		static constexpr auto HelpHintStorage = make_help_hint();

	public:
		static constexpr std::string_view HelpHint{ HelpHintStorage.data(), HelpHintStorage.size()};

		// A vector option parses one item per supplied argument.
		[[nodiscard]]
		static std::expected<TValue, std::string>
		parse(std::string_view text) {
			return ItemParser::parse(text);
		}
	};
}