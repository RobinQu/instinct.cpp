//
// Created by RobinQu on 2024/4/8.
//

#ifndef BASEFUNCTIONTOOL_HPP
#define BASEFUNCTIONTOOL_HPP

#include <utility>

#include "AgentGlobals.hpp"
#include "IFunctionTool.hpp"
#include <agent.pb.h>

#include "tools/ChronoUtils.hpp"

namespace INSTINCT_AGENT_NS {
    /**
     * Expects a `FunctionToolInvocation` message from context, and returns `FunctionToolResult` message.
     */
    class BaseFunctionTool: public virtual IFunctionTool, public BaseStepFunction {
        FunctionToolSchema schema_;

    public:
        explicit BaseFunctionTool(FunctionToolSchema schema)
            : schema_(std::move(schema)) {
        }

        [[nodiscard]] const FunctionToolSchema& GetSchema() const override {
            return schema_;
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            const auto invocation = input->RequireMessage<FunctionToolInvocation>();
            FunctionToolResult result;
            if (StringUtils::IsBlankString(invocation.id())) {
                result.set_invocation_id(StringUtils::GenerateUUIDString());
            } else {
                result.set_invocation_id(invocation.id());
            }
            LOG_DEBUG("Begin to function tool: name={},id={}", GetSchema().name(), result.invocation_id());
            const long t1 = ChronoUtils::GetCurrentTimeMillis();
            try {
                result.set_return_value(Execute(invocation.input()));
                LOG_DEBUG("Finish function tool: name={},id={},elapsed={}ms", GetSchema().name(), result.invocation_id(), ChronoUtils::GetCurrentTimeMillis() -  t1);
            } catch (const std::runtime_error& e) {
                result.set_exception(e.what());
                LOG_ERROR("Finish function tool with exception: name={},id={},elapsed={}ms, ex.waht={}", GetSchema().name(), result.invocation_id(), ChronoUtils::GetCurrentTimeMillis() -  t1, e.what());
            }
            input->ProduceMessage(result);
            return input;
        }

        virtual std::string Execute(const std::string& action_input) = 0;
    };

    using FunctionToolPtr = std::shared_ptr<BaseFunctionTool>;

}


#endif //BASEFUNCTIONTOOL_HPP
