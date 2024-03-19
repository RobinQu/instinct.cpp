//
// Created by RobinQu on 2024/2/1.
//

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <regex>
#include <ranges>
#include <string_view>
#include <string>
#include <iostream>
#include <unicode/ustream.h>
#include <Guid.hpp>
#include "Assertions.hpp"


namespace INSTINCT_CORE_NS {
    namespace u32_utils {
        static U32String copies_of(int n, const U32String& text) {
            U32String result;
            while (n-- > 0) {
                result += text;
            }
            return result;
        }

        static void print_splits(const std::string& announce, const std::vector<U32String>& splits,
                                 std::ostream& stream = std::cout, const bool flush = true) {
            stream << announce;
            for (const auto& f: splits) {
                stream << f << " | ";
            }
            if (flush) {
                stream << std::endl;
            }
        }
    }

    namespace u8_utils {
        /**
        * Generate UUID string, using system library. For windows, linux, and macs, different headers are concerned.
        *
        * https://stackoverflow.com/questions/543306/platform-independent-guid-generation-in-c
        *
        */
        static std::string uuid_v8() {
            auto g = xg::newGuid();
            // TODO this returns uuid string in uppercase, which should be in lowercase
            return g.str();
        }
    }


    struct StringUtils final {
        static std::string CopiesOf(int n, const std::string& text) {
            std::string result;
            while (n-- > 0) {
                result += text;
            }
            return result;
        }

        static std::vector<std::string> ReSplit(const std::string& s, const std::regex& sep_regex = std::regex{"\\s+"}) {
            std::sregex_token_iterator iter(s.begin(), s.end(), sep_regex, -1);
            std::sregex_token_iterator end;
            return {iter, end};
        }


        static std::string ToLower(const std::string_view& data) {
            auto parts = data | std::views::transform([](auto const c) { return std::tolower(c); });
            return {parts.begin(), parts.end()};
        }

        static std::string ToUpper(const std::string_view& data) {
            auto parts = data | std::views::transform([](auto const c) { return std::toupper(c); });
            return {parts.begin(), parts.end()};
        }


        static std::string Join(const std::vector<std::string>& parts) {
            std::string buf;
            for (const auto& part: parts) {
                buf += part;
            }
            return buf;
        }

        static std::string JoinWith(const sized_range auto& parts, const std::string& sep) {
            std::string buf;
            const size_t len = std::ranges::size(parts);
            for (int i = 0; const auto& part: parts) {
                buf += part;
                if (++i < len) {
                    buf += sep;
                }
            }
            return buf;
        }

        static std::string Trim(const std::string& input) {
            auto s = input;
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());

            return s;
        }

        static bool IsBlankString(const std::string& s) {
            return s.empty() || Trim(s).empty();
        }

        static std::string GetWithDefault(const std::string& value, const std::string& fallback) {
            if (IsBlankString(value)) {
                return fallback;
            }
            return value;
        }
    };
}

#endif //STRINGUTILS_H
