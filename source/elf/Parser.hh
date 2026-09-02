#pragma once

#include "elf/Object.hh"

#include <expected>
#include <filesystem>
#include <istream>
#include <string>
#include <string_view>
#include <vector>

namespace Revo::ELF {

[[nodiscard]] std::expected<Object, std::string>
parse(const std::filesystem::path& path);

[[nodiscard]] std::expected<Object, std::string>
parse(std::istream& stream);

namespace Impl {

// Parsing steps
[[nodiscard]] std::expected<void, std::string>
read_elf_header(Object& object, std::istream& stream);

[[nodiscard]] std::expected<void, std::string>
read_sections(Object& object, std::istream& stream);

[[nodiscard]] std::expected<void, std::string>
read_section_names(Object& object);

[[nodiscard]] std::expected<void, std::string>
read_symbols(Object& object);

[[nodiscard]] std::expected<void, std::string>
read_revo_relocations(Object& object);

[[nodiscard]] std::expected<void, std::string>
read_revo_functions(Object& object);

[[nodiscard]] std::expected<void, std::string>
check_functions(const Object& object)
    pre(std::ranges::is_sorted(object.revo_functions, {}, &Function::offset));

[[nodiscard]] std::expected<void, std::string>
check_relocations(const Object& object)
    pre(std::ranges::is_sorted(object.revo_functions, {}, &Function::offset));

// Utility functions
[[nodiscard]] std::expected<std::string_view, std::string>
read_string(const Section& section, u32 offset);

template <typename TType>
[[nodiscard]] std::expected<std::vector<TType>, std::string>
read_table(const Section& section);

} // namespace Impl

} // namespace Revo::ELF
