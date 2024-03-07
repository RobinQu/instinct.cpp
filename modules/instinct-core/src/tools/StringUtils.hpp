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
#include <uuid/uuid.h>


namespace INSTINCT_CORE_NS {

    namespace u32_utils {
        static U32String copies_of(int n, const U32String& text) {
            U32String result;
            while(n-->0) {
                result += text;
            }
            return result;
        }

        static void print_splits(const std::string& announce , const std::vector<U32String>& splits, std::ostream& stream = std::cout, const bool flush = true) {
            std::cout << announce;
            for(const auto& f: splits) {
                stream << f << " | ";
            }
            if(flush) {
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
            uuid_t uuid;
            uuid_generate_random ( uuid );
            char s[37];
            uuid_unparse ( uuid, s );
            return s;
        }
    }


    struct StringUtils final {

        static std::string CopiesOf(int n, const std::string& text) {
            std::string result;
            while(n-->0) {
                result += text;
            }
            return result;
        }



        static std::vector<std::string> Resplit(const std::string &s, const std::regex &sep_regex = std::regex{"\\s+"}) {
            std::sregex_token_iterator iter(s.begin(), s.end(), sep_regex, -1);
            std::sregex_token_iterator end;
            return {iter, end};
        }


        static std::string ToLower(const std::string_view& data) {
            auto parts =  data | std::views::transform([](auto const c) {return std::tolower(c);});
            return {parts.begin(), parts.end()};
        }

        static std::string ToUpper(const std::string_view& data) {
            auto parts =  data | std::views::transform([](auto const c) {return std::toupper(c);});
            return {parts.begin(), parts.end()};
        }


        static std::string Join(const std::vector<std::string>& parts) {
            std::string buf;
            for(const auto& part: parts) {
                buf += part;
            }
            return buf;
        }


        static std::string JoinWith(const sized_range auto& parts, const std::string& sep) {
            std::string buf;
            const size_t len = std::ranges::size(parts);
            for(int i=0; const auto& part: parts) {
                buf+= part;
                if(++i < len) {
                    buf+= sep;
                }
            }
            return buf;
        }

        static std::string Trim(const std::string& input) {
            auto view = input | std::views::filter([](unsigned char c) {
               return !std::isspace(c);
            });
            return std::string {view.begin(), view.end()};
        }



    };


}

#endif //STRINGUTILS_H
