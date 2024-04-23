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
#include <unicode/regex.h>


namespace INSTINCT_CORE_NS {
    using namespace U_ICU_NAMESPACE;




    class U32StringUtils {
    public:
        static U32String CopiesOf(int n, const UnicodeString& text) {
            U32String result;
            while (n-- > 0) {
                result += text;
            }
            return result;
        }



        /**
         * split with regex pattern string
         * @tparam max_split_size
         * @param text
         * @param seperator
         * @param result
         */
        template<int max_split_size=3>
        static void SpilitWithRegex(const UnicodeString& text, const UnicodeString& seperator, std::vector<UnicodeString>& result) {
            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher matcher(seperator, 0, status);
            if(U_FAILURE(status)) {
                std::string sep_utf8;
                throw InstinctException("Failed to compile regex with seperator string: " + seperator.toUTF8String(sep_utf8));
            }
            UnicodeString parts[max_split_size];
            // we do exhaustive splitting using do-while loop
            int32_t splits_size = 0;
            auto text_to_be_split = text;
            // std::cout << "input=" << text_to_be_split << std::endl;
            do {
                // TODO: fix needed! sometimes last chunk of remaining split is ill-formed resulting incomplete text return.
                splits_size = matcher.split(text_to_be_split, parts, max_split_size, status);
                if(U_FAILURE(status)) {
                    throw InstinctException("Failed to split text with seperator regex");
                }
                if (splits_size>0) {
                    result.insert(result.end(), parts, parts+splits_size-1);
                    text_to_be_split = parts[splits_size-1];
                }
            } while(max_split_size == splits_size);

            result.push_back(parts[splits_size-1]);
        }
    };


    struct StringUtils final {

        /**
        * Generate UUID string, using system library. For windows, linux, and macs, different headers are concerned.
        *
        * https://stackoverflow.com/questions/543306/platform-independent-guid-generation-in-c
        *
        */
        static std::string GenerateUUIDString() {
            auto g = xg::newGuid();
            return g.str();
        }

        static std::string CopiesOf(int n, const std::string& text) {
            std::string result;
            while (n-- > 0) {
                result += text;
            }
            return result;
        }

        static std::vector<std::smatch> MatchPattern(const std::string& paragraph, const std::regex& pattern) {
            std::sregex_iterator iter(paragraph.begin(), paragraph.end(), pattern);
            const std::sregex_iterator end;
            std::vector<std::smatch> matches;
            while (iter != end) {
                matches.push_back(*iter++);
            }
            return matches;
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

        static bool IsNotBlankString(const std::string& s) {
            return !IsBlankString(s);
        }

        static std::string GetWithDefault(const std::string& value, const std::string& fallback) {
            if (IsBlankString(value)) {
                return fallback;
            }
            return value;
        }

        static std::string SnakeToCamel(const std::string& str) {
            // Empty String
            std::string result;

            // Append first character(in lower case)
            // to result string
            result += static_cast<char>(std::tolower(static_cast<unsigned char>(str[0])));

            // Traverse the string from
            // ist index to last index
            for (int i = 1; i < str.length(); i++) {
                char ch = str[i];
                // Check if the character is upper case
                // then append '_' and such character
                // (in lower case) to result string
                if (std::isupper(ch)) {
                    result += '_';
                    result += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                }

                // If the character is lower case then
                // add such character into result string
                else {
                    result += ch;
                }
            }

            // return the result
            return result;
        }


        /**
         * Esacpe string for sql text value
         * @sa https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-escape-string.html
         * @sa https://github.com/kylefarris/tsqlstring/blob/master/lib/SqlString.js
         * @param s input string to escape
         * @return escaped string that's safe for sql text value
         */
        static std::string EscapeSQLText(const std::string& s) {
            const std::unordered_map<char, std::string> CHARS_ESCAPE_MAP {
                    {'\0' , "\\0"},
                    {'\b'   , "\\b"},
                    {'\f'   , "\\f"},
                    {'\t'   , "\\t"},
                    {'\n'   , "\\n"},
                    {'\r'   , "\\r"},
                    {'\v'   , "\\v"},
                    {'\x1a' , "\\Z"},
                    {'\''    , "\'\'"},
                    {'\\' , "\\\\"}
            };
            std::string copied;
            for(const auto c: s) {
                if (CHARS_ESCAPE_MAP.contains(c)) {
                    copied += CHARS_ESCAPE_MAP.at(c);
                } else {
                    copied += c;
                }
            }
            return copied;
        }
    };


    static void assert_not_blank(const std::string& v, const std::string& msg = "given string cannot be blank") {
        if (StringUtils::IsBlankString(v)) {
            throw InstinctException(msg);
        }
    }
}

#endif //STRINGUTILS_H
