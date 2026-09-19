#include "ppc/Mnemonic.hh"
#include "util/Util.hh"
#include "ppc/Register.hh"

#include <meta>

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>

using namespace Revo;

namespace {

template <typename TEnum>
void
add_enum(nanobind::handle handle) {
    auto enumeration = nanobind::enum_<TEnum>(handle, Util::identifier_string(^^TEnum));

    template for (constexpr auto enumerator : Util::enumerators_array(^^TEnum)) {
        enumeration.value(Util::identifier_string(enumerator), [:enumerator:]);
    }
}

} // namespace

NB_MODULE(revo, module) {
    auto ppc = module.def_submodule(Util::identifier_string(^^PPC));
    add_enum<PPC::Mnemonic>(ppc);

    auto ppc_register = ppc.def_submodule(Util::identifier_string(^^PPC::Register));
    add_enum<PPC::Register::GPR>(ppc_register);
    add_enum<PPC::Register::FPR>(ppc_register);
    add_enum<PPC::Register::SPR>(ppc_register);
    add_enum<PPC::Register::CR>(ppc_register);
}
