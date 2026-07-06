#pragma once

#include <cstdlib>
#include <print>
#include <source_location>
#include <string_view>

namespace {

inline void
runtime_assert_fail(std::string_view condition, std::string_view message,
    std::source_location loc = std::source_location::current()) {
    std::print(stderr,
        "ASSERTION FAILED: ({})\n"
        "\tMessage: {}\n"
        "\tFile: {}:{}\n"
        "\tFunction: {}\n",
        condition, message, loc.file_name(), loc.line(), loc.function_name());
    std::abort();
}

} // namespace

#define STATIC_ASSERT(cond, msg) static_assert((cond), msg)

#define RUNTIME_ASSERT(cond, msg) \
    do { \
        if (!(cond)) [[unlikely]] { \
            runtime_assert_fail(#cond, msg); \
        } \
    } while (false)
