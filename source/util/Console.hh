#pragma once

#include <atomic>
#include <format>
#include <print>
#include <string_view>
#include <utility>

namespace Revo {

class Console {
public:
    enum class LogLevel : u8 {
        None = 0,
        Error,
        Info,
        Debug,
    };

    static void
    set_level(LogLevel level) {
        mLogLevel.store(level, std::memory_order_relaxed);
    }

    static LogLevel
    get_level() {
        return mLogLevel.load(std::memory_order_relaxed);
    }

    template <typename... TArgs>
    static void
    error(std::format_string<TArgs...> string, TArgs&&... args) {
        if (get_level() >= LogLevel::Error) {
            std::println(stderr, "{}[ERROR]{}   {}", ERROR_COLOR, RESET_COLOR,
                std::format(string, std::forward<TArgs>(args)...));
        }
    }

    template <typename... TArgs>
    static void
    info(std::format_string<TArgs...> string, TArgs&&... args) {
        if (get_level() >= LogLevel::Info) {
            std::println(stdout, "{}[INFO]{}    {}", INFO_COLOR, RESET_COLOR,
                std::format(string, std::forward<TArgs>(args)...));
        }
    }

    template <typename... TArgs>
    static void
    debug(std::format_string<TArgs...> string, TArgs&&... args) {
        if (get_level() >= LogLevel::Debug) {
            std::println(stdout, "{}[DEBUG]{}   {}", DEBUG_COLOR, RESET_COLOR,
                std::format(string, std::forward<TArgs>(args)...));
        }
    }

    template <typename... TArgs>
    static void
    success(std::format_string<TArgs...> string, TArgs&&... args) {
        if (get_level() >= LogLevel::Info) {
            std::println(stdout, "{}[SUCCESS]{} {}", SUCCESS_COLOR, RESET_COLOR,
                std::format(string, std::forward<TArgs>(args)...));
        }
    }

private:
    static constexpr std::string_view ERROR_COLOR = "\033[38;2;255;255;255m\033[48;2;112;0;0m";
    static constexpr std::string_view INFO_COLOR = "\033[38;2;255;255;255m\033[48;2;0;78;109m";
    static constexpr std::string_view DEBUG_COLOR = "\033[38;2;255;255;255m\033[48;2;107;0;107m";
    static constexpr std::string_view SUCCESS_COLOR = "\033[38;2;255;255;255m\033[48;2;17;104;0m";
    static constexpr std::string_view RESET_COLOR = "\033[0m";

    static inline std::atomic<LogLevel> mLogLevel{LogLevel::Info};
};

} // namespace Revo
