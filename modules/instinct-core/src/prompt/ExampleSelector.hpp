//
// Created by RobinQu on 2024/2/14.
//

#ifndef BASEEXMAPLESELECTOR_H
#define BASEEXMAPLESELECTOR_H
#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"


namespace INSTINCT_CORE_NS {

class ExampleSelector {

public:
    // explicit BaseExmapleSelector(std::vector<OptionDict> examples);
    virtual ~ExampleSelector() = default;
    virtual void AddExample(const OptionDict& example) = 0;
    virtual std::vector<OptionDict> SelectExamples(const OptionDict& variables) = 0;
    [[nodiscard]] virtual  const std::vector<OptionDict>& GetAllExamples() const = 0;
};


} // namespace INSTINCT_CORE_NS

#endif //BASEEXMAPLESELECTOR_H
