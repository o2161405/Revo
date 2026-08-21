#pragma once

namespace Revo::CLI {

template <typename TSpecification>
concept HasArguments = requires { TSpecification::arguments; };

template <typename TSpecification>
concept HasType = requires { typename TSpecification::Type; };

template <typename TSpecification>
concept HasRequired = requires { TSpecification::required; };

template <typename TSpecification>
concept HasDescription = requires { TSpecification::description; };

} // namespace Revo::CLI
