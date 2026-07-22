#pragma once

#include <algorithm>   // std::ranges::find_first_of
#include <array>       // std::array
#include <cstddef>     // std::size_t
#include <cstdlib>     // std::system
#include <format>      // std::format
#include <print>       // std::println
#include <ranges>      // views, ranges::to
#include <span>        // std::span
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::exchange
#include <vector>      // std::vector

namespace Revo::Util {
    struct Shell {
        inline constexpr std::string_view program_name() const {
            return m_args.empty() ? std::string_view{} : m_args.at(0);
        }

        inline constexpr std::span<const std::string> user_args() const {
            if (m_args.size() <= 1) {
                return {};
            }
            return view().subspan(1);
        }

        inline constexpr std::span<const std::string> view() const {
            return { m_args };
        }

        // Escapes and joins the arguments into the command-line text used for execution and backend generation.
        inline constexpr std::string full_command() const {
            return m_args
                | std::views::transform(arg_escape)
                | std::views::join_with(' ')
                | std::ranges::to<std::string>();
        }

        auto invoke(bool verbose, bool dry_run) const {
            const auto command = full_command();

            if (verbose or dry_run) {
                std::println("{}", command);
            }

            if (dry_run) {
                return 0;
            }

#ifdef _WIN32
            // cmd.exe strips outer quotes for /c; keep inner quotes around a spaced executable path intact.
            return std::system(std::format("\"{}\"", command).c_str());
#else
            return std::system(command.c_str());
#endif
        }

        // Returns true when an argument needs escaping before being written into a
        // generated shell command.
        //
        // This is required for empty arguments, whitespace, quotes, and characters that
        // may affect how the shell interprets the command line.
        static inline constexpr bool arg_needs_escape(std::string_view arg) {
            return arg.empty() or (std::ranges::find_first_of(arg, special_characters) != arg.end());
        }

        static inline constexpr std::array special_characters{
#ifdef _WIN32
        ' ', '\t', '"', '&', '|', '<', '>', '^', '%', '!', '(', ')'
#else
        ' ', '\t', '\'', '"', '$', '\\', '&', ';', '(', ')', '<', '>', '|'
#endif // _WIN32
        };

        static inline constexpr std::string_view empty_arg_escape{
#ifdef _WIN32
            "\"\""
#else
            "''"
#endif // _WIN32
        };

        static inline constexpr char quote_character{
#ifdef _WIN32
            '"'
#else
            '\''
#endif // _WIN32
        };

        // Escapes one command-line argument for use in a generated shell command.
        //
        // This is only for individual arguments, not whole commands, pipelines,
        // redirections, or multiple concatenated arguments.
        static inline constexpr std::string arg_escape(std::string_view arg) {
            if (not arg_needs_escape(arg)) {
                return std::string{ arg };
            }

            if (arg.empty()) {
                return std::string{ empty_arg_escape };
            }

            std::string result;
            result.reserve(arg.size() + 2);
            result += quote_character;

#ifdef _WIN32
            static constexpr char escape_character = '\\';
            std::size_t backslashes = 0;

            for (const char ch : arg) {
                if (ch == escape_character) {
                    ++backslashes;
                    continue;
                }

                if (ch == quote_character) {
                    result.append((std::exchange(backslashes, 0) * 2) + 1, escape_character);
                    result.push_back(quote_character);
                    continue;
                }

                result.append(std::exchange(backslashes, 0), escape_character);
                result.push_back(ch);
            }

            result.append(backslashes * 2, escape_character);
#else
            static constexpr std::string_view posix_quote_escape = "'\\''";

            auto escaped_body = arg
                | std::views::split(quote_character)
                | std::views::join_with(posix_quote_escape);

            result.append_range(escaped_body);
#endif // _WIN32

            result += quote_character;
            return result;
        }

        // Copies the already-tokenized argc/argv arguments; escaping is handled when command text is generated.
        static inline constexpr Shell from_args(int argc, const char* const* argv) {
            if (argc <= 0 or argv == nullptr) {
                return {};
            }

            return {
                std::views::counted(argv, argc)
                | std::ranges::to<std::vector<std::string>>()
            };
        }

        std::vector<std::string> m_args{};
    };
}