#pragma once

#include "../FixedString.hh"

#include <string_view>

namespace Revo::Util {
	// Option descriptor
	// for use at compile time
	template<
		FixedString VName = "",  // Option name (long switch)
		FixedString VDesc = "",  // Option description (for help menu or smth)
		char VShort = '\0',  // Optional short switch char
		auto FnCallback = nullptr,  // Optional callback, nullptr for none
		FixedString VKey = "",  // Key for value in option storage, empty for none (ignores TValue)
		typename TValue = void,  // Type for value in option storage, void for none
		bool VIsPositional = false,  // Whether this is a positional argument instead of a named switch
		bool VIsHidden = false // If the option is not meant to be seen or used by the user (usually for option storage values)
	>
	struct OptionDesc {
		template<FixedString VNewName>
		consteval auto name() const {
			return OptionDesc<VNewName, VDesc, VShort, FnCallback, VKey, TValue, VIsPositional, VIsHidden>{};
		}

		static constexpr std::string_view name() {
			return VName;
		}

		template<FixedString VNewDesc>
		consteval auto description() const {
			return OptionDesc<VName, VNewDesc, VShort, FnCallback, VKey, TValue, VIsPositional, VIsHidden>{};
		}

		static constexpr std::string_view description() {
			return VDesc;
		}

		template<char VNewShort>
		consteval auto short_name() const {
			return OptionDesc<VName, VDesc, VNewShort, FnCallback, VKey, TValue, VIsPositional, VIsHidden>{};
		}

		static constexpr char short_name() {
			return VShort;
		}

		// NOTE: Callbacks are invoked during parse
		template<auto VNewCallback>
		consteval auto callback() const {
			return OptionDesc<VName, VDesc, VShort, VNewCallback, VKey, TValue, VIsPositional, VIsHidden>{};
		}

		static constexpr decltype(auto) callback() {
			return FnCallback;
		}

		template<FixedString VNewKey, typename TNewValue>
		consteval auto storage_info() const {
			return OptionDesc<VName, VDesc, VShort, FnCallback, VNewKey, TNewValue, VIsPositional, VIsHidden>{};
		}

		static constexpr auto StorageKey = VKey;
		static constexpr std::string_view storage_key() {
			return StorageKey.view();
		}

		template<bool VNewIsPositional = true>
		consteval auto positional() const {
			return OptionDesc<VName, VDesc, VShort, FnCallback, VKey, TValue, VNewIsPositional, VIsHidden>{};
		}
		static constexpr bool IsPositional = VIsPositional;

		template<bool VNewIsHidden = true>
		consteval auto hidden() const {
			return OptionDesc<VName, VDesc, VShort, FnCallback, VKey, TValue, VIsPositional, VNewIsHidden>{};
		}
		static constexpr bool IsHidden = VIsHidden;

		using ValueType = TValue;
		using Type = OptionDesc;
	};

	static constexpr auto Option = OptionDesc{};
}