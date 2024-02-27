//
// Created by RobinQu on 2024/2/12.
//

#ifndef GENERATION_H
#define GENERATION_H
#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"

namespace INSTINCT_CORE_NS {

struct Generation {

    std::string text;
    OptionDict generation_info;
    // explicit Generation(std::string text, OptionDict generation_info, std::string type)
    //     : text(std::move(text)), generation_info(std::move(generation_info)), type(std::move(type)) {
    // }
};

} // namespace INSTINCT_CORE_NS

#endif //GENERATION_H
