//
// Created by RobinQu on 2024/4/18.
//

#ifndef IOUTILS_HPP
#define IOUTILS_HPP

#include <string>
#include <fstream>
#include <streambuf>

#include "CoreGlobals.hpp"


namespace INSTINCT_CORE_NS {
    class IOUtils final {
    public:
        static std::string ReadString(const std::filesystem::path& file_path) {
            std::ifstream t(file_path);
            std::string str;

            t.seekg(0, std::ios::end);
            str.reserve(t.tellg());
            t.seekg(0, std::ios::beg);

            str.assign((std::istreambuf_iterator<char>(t)),
                        std::istreambuf_iterator<char>());
            return str;
        }
    };
}
#endif //IOUTILS_HPP
