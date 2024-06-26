//
// Created by RobinQu on 2024/3/8.
//

#ifndef OUTPUTPARSER_HPP
#define OUTPUTPARSER_HPP


#include <llm.pb.h>

#include <instinct/LLMGlobals.hpp>
#include <instinct/functional/json_context.hpp>

namespace INSTINCT_LLM_NS {

    template<typename T>
    class IOutputParser{
    public:
        IOutputParser()=default;
        virtual ~IOutputParser()=default;
        IOutputParser(IOutputParser&&)=delete;
        IOutputParser(const IOutputParser&)=delete;

        virtual T ParseResult(
            const Generation& generation
        ) = 0;

        virtual std::string GetFormatInstruction() = 0;
    };

    template<typename T>
    using OutputParserLambda = std::function<T(const JSONContextPtr& context)>;

}

#endif //OUTPUTPARSER_HPP
