//
// Created by RobinQu on 2024/4/9.
//

#ifndef BASEWORKER_HPP
#define BASEWORKER_HPP

#include "AgentGlobals.hpp"
#include "toolkit/BaseFunctionToolkit.hpp"

namespace INSTINCT_AGENT_NS {
    class BaseWorker: public BaseRunnable<AgentThoughtMessage, AgentObservationMessage> {
        std::vector<FunctionToolkitPtr> toolkits_;
    public:
        explicit BaseWorker(const std::vector<FunctionToolkitPtr> &toolkits)
            : toolkits_(toolkits) {
        }

        /**
         * Get available `FunctionToolKit` list
         * @return
         */
        [[nodiscard]] const std::vector<FunctionToolkitPtr>& GetFunctionToolkits() const {
            return toolkits_;
        }
    };
    using WorkerPtr = std::shared_ptr<BaseWorker>;
}

#endif //BASEWORKER_HPP
