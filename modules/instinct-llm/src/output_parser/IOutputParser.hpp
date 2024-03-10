//
// Created by RobinQu on 2024/3/8.
//

#ifndef OUTPUTPARSER_HPP
#define OUTPUTPARSER_HPP


#include <llm.pb.h>

#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {

    template<typename T>
    class IOutputParser {
    public:
        IOutputParser()=default;
        virtual ~IOutputParser()=default;
        IOutputParser(IOutputParser&&)=delete;
        IOutputParser(const IOutputParser&)=delete;

        virtual T ParseResult(
            const std::string& result
        ) = 0;
    };

    template<typename T>
    using OutputParserPtr = std::shared_ptr<IOutputParser<T>>;

}

#endif //OUTPUTPARSER_HPP
