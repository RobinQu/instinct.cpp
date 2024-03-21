//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_BASERUNNABLE_HPP
#define INSTINCT_BASERUNNABLE_HPP

#include "CoreGlobals.hpp"
#include "IRunnable.hpp"

namespace INSTINCT_CORE_NS {

    template<typename Input,typename Output>
    class BaseRunnable: public virtual IRunnable<Input,Output> {

    };








}




#endif //INSTINCT_BASERUNNABLE_HPP
