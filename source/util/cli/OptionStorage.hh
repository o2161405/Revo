#pragma once

#include <array>
#include <cstddef>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Revo::Util {
	template<typename TOptions>
	struct OptionStorage;

	template<typename... TOpts>
	struct OptionStorage<std::tuple<TOpts...>> {
	private:
		// We don't want to use the options tuple for storage,
		// but a new one that can map the option keys to the values
		// This is because some options might want to share a key and set
		// different values
		using OptionsType = std::tuple<TOpts...>;

		static constexpr std::size_t OptionCount = sizeof...(TOpts);

		template<std::size_t VIndex>
		using OptionType = std::tuple_element_t<VIndex, OptionsType>;

		// Includes empty keys and duplicate keys.
		static constexpr std::array<std::string_view, OptionCount> AllKeys{
			TOpts::storage_key()...
		};

		// True when the option supplies the actual type for a storage key.
		static constexpr std::array<bool, OptionCount> DefinesStorage{
			(
				not TOpts::storage_key().empty()
				and not std::is_void_v<typename TOpts::ValueType>
			)...
		};

		// The first option defining a key owns that key's tuple slot.
		template<std::size_t VIndex>
		static consteval bool owns_storage() {
			using OptType = OptionType<VIndex>;

			if constexpr (std::is_void_v<typename OptType::ValueType>) {
				return false;
			}

			constexpr std::string_view key = OptType::storage_key();

			if (key.empty()) {
				return false;
			}

			for (std::size_t previous = 0; previous < VIndex; ++previous) {
				if (DefinesStorage[previous] and AllKeys[previous] == key) {
					return false;
				}
			}

			return true;
		}

		// Produces tuple<ValueType> for an owning option or tuple<> otherwise.
		// The if constexpr prevents tuple<void> from being instantiated.
		template<std::size_t VIndex>
		static consteval auto storage_piece_type() {
			if constexpr (owns_storage<VIndex>()) {
				return std::type_identity<std::tuple<typename OptionType<VIndex>::ValueType>>{};
			} else {
				return std::type_identity<std::tuple<>>{};
			}
		}

		template<std::size_t... VIndices>
		static auto make_storage_type(std::index_sequence<VIndices...>) -> decltype(
			std::tuple_cat(std::declval<typename decltype(storage_piece_type<VIndices>())::type>()...)
		);

		template<std::size_t... VIndices>
		static consteval std::size_t storage_size(std::index_sequence<VIndices...>) {
			return (std::size_t{0} + ... + (owns_storage<VIndices>() ? std::size_t{1} : std::size_t{0}));
		}

		static constexpr std::size_t StorageSize = storage_size(std::make_index_sequence<OptionCount>{});

		template<std::size_t VIndex>
		static consteval void append_key(std::array<std::string_view, StorageSize>& result, std::size_t& output_index) {
			if constexpr (owns_storage<VIndex>()) {
				result[output_index++] = AllKeys[VIndex];
			}
		}

		template<std::size_t... VIndices>
		static consteval auto make_keys(std::index_sequence<VIndices...>) {
			std::array<std::string_view, StorageSize> result{};
			std::size_t output_index = 0;

			(append_key<VIndices>(result, output_index), ...);

			return result;
		}

	public:
		static constexpr auto keys = make_keys(std::make_index_sequence<OptionCount>{});
		using StorageType = decltype(make_storage_type(std::make_index_sequence<OptionCount>{}));

private:
		template<FixedString VKey>
		static consteval std::size_t storage_index() {
			constexpr std::string_view key = VKey.view();

			for (std::size_t index = 0; index < keys.size(); ++index) {
				if (keys[index] == key) {
					return index;
				}
			}

			return keys.size();
		}

public:
		/*
		 * Key overloads
		 */

		template<FixedString VKey>
		static consteval bool has() {
			return storage_index<VKey>() < keys.size();
		}

		template<FixedString VKey>
		constexpr decltype(auto) get() {
			constexpr std::size_t index = storage_index<VKey>();

			static_assert(index < keys.size(), "OptionStorage does not contain the requested key");

			return (std::get<index>(mStorage));
		}

		template<FixedString VKey>
		constexpr decltype(auto) get() const {
			constexpr std::size_t index = storage_index<VKey>();

			static_assert(index < keys.size(), "OptionStorage does not contain the requested key");

			return (std::get<index>(mStorage));
		}

		template<FixedString VKey, typename TValue>
		constexpr void set(TValue&& value) {
			constexpr std::size_t index = storage_index<VKey>();

			static_assert(index < keys.size(), "OptionStorage does not contain the requested key");

			std::get<index>(mStorage) = std::forward<TValue>(value);
		}

		/*
		 * Option-type overloads
		 */

		template<typename TOpt>
		static consteval bool has() {
			return has<TOpt::StorageKey>();
		}

		template<typename TOpt>
		constexpr decltype(auto) get() {
			static_assert(has<TOpt>(), "Missing storage key for option");

			return this->template get<TOpt::StorageKey>();
		}

		template<typename TOpt>
		constexpr decltype(auto) get() const {
			static_assert(has<TOpt>(), "Missing storage key for option");

			return this->template get<TOpt::StorageKey>();
		}

		template<typename TOpt, typename TValue>
		constexpr void set(TValue&& value) {
			static_assert(has<TOpt>(), "Missing storage key for option");

			this->template set<TOpt::StorageKey>(std::forward<TValue>(value));
		}

		StorageType mStorage{};
	};
}