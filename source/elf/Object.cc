#include "Object.hh"

#include <algorithm>

namespace Revo::ELF {

std::optional<const Section&>
Object::get_section(std::string_view name) const {
    if (const auto it = std::ranges::find(sections, name, &Section::name); it != sections.end()) {
        return *it;
    }

    return std::nullopt;
}

std::optional<const Symbol&>
Object::get_symbol(std::string_view name) const {
    if (const auto it = std::ranges::find(symbols, name, &Symbol::name); it != symbols.end()) {
        return *it;
    }

    return std::nullopt;
}

} // namespace Revo::ELF
