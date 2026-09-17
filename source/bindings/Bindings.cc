#include "ppc/Mnemonic.hh"
#include "ppc/Register.hh"
#include "util/Util.hh"

#include <meta>

#include "bindings/Types.hh"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>

namespace Revo::Bindings {

NB_MODULE(revo, module) {
    auto ppc = module.def_submodule(Util::identifier_string(^^PPC));
    Enum<PPC::Mnemonic>::make(ppc);

    auto ppc_register = ppc.def_submodule(Util::identifier_string(^^PPC::Register));
    Enum<PPC::Register::GPR>::make(ppc_register);
    Enum<PPC::Register::FPR>::make(ppc_register);
    Enum<PPC::Register::SPR>::make(ppc_register);
    Enum<PPC::Register::CR>::make(ppc_register);
}

} // namespace Revo::Bindings
