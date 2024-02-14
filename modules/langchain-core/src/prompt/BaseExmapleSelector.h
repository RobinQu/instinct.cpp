//
// Created by RobinQu on 2024/2/14.
//

#ifndef BASEEXMAPLESELECTOR_H
#define BASEEXMAPLESELECTOR_H
#include "CoreGlobals.h"
#include "CoreTypes.h"


namespace LC_CORE_NS {

class BaseExmapleSelector {

public:
    // explicit BaseExmapleSelector(std::vector<OptionDict> examples);
    virtual ~BaseExmapleSelector() = default;
    virtual void AddExample(const OptionDict& example) = 0;
    virtual std::vector<OptionDict> SelectExamples(const OptionDict& variables) = 0;
    [[nodiscard]] virtual  const std::vector<OptionDict>& GetAllExamples() const = 0;
};

using BaseExampleSelectorPtr = std::shared_ptr<BaseExmapleSelector>;

} // LC_CORE_NS

#endif //BASEEXMAPLESELECTOR_H
