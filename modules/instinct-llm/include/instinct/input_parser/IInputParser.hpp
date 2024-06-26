//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_IINPUTPARSER_HPP
#define INSTINCT_IINPUTPARSER_HPP

#include <instinct/LLMGlobals.hpp>
#include <instinct/functional/json_context.hpp>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    template<typename T>
    class IInputParser {
    public:
        virtual ~IInputParser() = default;

        virtual JSONContextPtr ParseInput(const T& input) = 0;
    };

    template<typename T>
    using InputParserLambda = std::function<JSONContextPtr(const T& input)>;

}

#endif //INSTINCT_IINPUTPARSER_HPP
