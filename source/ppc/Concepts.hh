#pragma once

#include <meta>

namespace Revo::PPC {

template <typename TField>
concept IsExtendedOpcodeField = requires { requires TField::is_extended_opcode; };

template <typename TField>
concept IsOperandField = requires { requires TField::is_operand_field; };

template <typename TField>
concept IsRegisterField = requires { requires TField::is_register_field; };

template <typename TField>
concept IsBehaviorField = requires { requires TField::is_behavior_field; };

template <typename TSpecification, typename TField>
concept IsZeroableField = requires {
    requires std::meta::dealias(TSpecification::zeroable_field) == std::meta::dealias(^^TField);
};

template <typename TSpecification>
concept HasConstants = requires { TSpecification::constants; };

template <typename TSpecification>
concept HasAccesses = requires { TSpecification::accesses; };

template <typename TSpecification>
concept HasZeroableField = requires { TSpecification::zeroable_field; };

template <typename TSpecification>
concept HasImpliedBehaviors = requires { TSpecification::implied_behaviors; };

template <typename TSpecification>
concept HasIndirectBranchSource = requires { TSpecification::indirect_branch_source; };

template <typename TSpecification>
concept HasExtendedOpcode = requires { TSpecification::xo; };

} // namespace Revo::PPC
