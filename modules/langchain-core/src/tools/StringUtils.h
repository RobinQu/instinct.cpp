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


namespace langchian::core {
    struct StringUtils final {
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

        static std::string JoinWith(const std::vector<std::string>& parts, const std::string& sep) {
            std::string buf;
            const size_t len = parts.size();
            for(int i=0;i<len;++i) {
                buf+= parts[i];
                if(i!=len-1) {
                    buf+= sep;
                }
            }
            return buf;
        }



    };


}

#endif //STRINGUTILS_H
