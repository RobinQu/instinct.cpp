//
// Created by RobinQu on 2024/3/8.
//

#ifndef OUTPUTPARSER_HPP
#define OUTPUTPARSER_HPP


#include <llm.pb.h>

#include "RetrievalGlobals.hpp"


namespace INSTINCT_LLM_NS {

    template<typename T>
    // requires is_pb_message<T>
    class IOutputParser {
    public:
        IOutputParser()=default;
        virtual ~IOutputParser()=default;
        IOutputParser(IOutputParser&&)=delete;
        IOutputParser(const IOutputParser&)=delete;

        virtual T ParseResult(
            const LangaugeModelOutput& model_output
        ) = 0;
    };

    template<typename T>
    using OutputParserPtr = std::shared_ptr<IOutputParser<T>>;

}

#endif //OUTPUTPARSER_HPP
