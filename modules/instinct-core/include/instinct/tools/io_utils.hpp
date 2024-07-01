//
// Created by RobinQu on 2024/4/18.
//

#ifndef IOUTILS_HPP
#define IOUTILS_HPP

#include <string>
#include <fstream>
#include <streambuf>
#include <instinct/tools/assertions.hpp>
#include <instinct/core_global.hpp>


namespace INSTINCT_CORE_NS {
    class IOUtils final {
    public:
        /**
         * Read file into string at your own risks
         * @sa https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
         * @sa https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
         * @param file_path
         * @return
         */
        static std::string ReadString(const std::filesystem::path& file_path) {
            assert_true(std::filesystem::exists(file_path), "file should exist at " + file_path.string());
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
