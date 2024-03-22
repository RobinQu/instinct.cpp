//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_BASEINPUTPARSER_HPP
#define INSTINCT_BASEINPUTPARSER_HPP

#include "IInputParser.hpp"
#include "functional/BaseRunnable.hpp"
#include "functional/JSONContextPolicy.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    template<typename T>
    class BaseInputParser: public virtual IInputParser<T>, public BaseRunnable<T,JSONContextPtr> {
    public:
        JSONContextPtr Invoke(const T &input) override {
            return this->ParseInput(input);
        }
    };
    template<typename T>
    using InputParserPtr = std::shared_ptr<BaseInputParser<T>>;
}

#endif //INSTINCT_BASEINPUTPARSER_HPP
