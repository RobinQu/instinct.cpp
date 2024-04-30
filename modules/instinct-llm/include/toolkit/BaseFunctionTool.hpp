//
// Created by RobinQu on 2024/4/8.
//

#ifndef BASEFUNCTIONTOOL_HPP
#define BASEFUNCTIONTOOL_HPP

#include "LLMGlobals.hpp"
#include "IFunctionTool.hpp"
#include <agent.pb.h>

#include "tools/ChronoUtils.hpp"
#include "tools/ProtobufUtils.hpp"

namespace INSTINCT_LLM_NS {
    struct FunctionToolOptions {
        /**
         * A postive value will trigger retry in `Invoke` method.
         */
        uint8_t max_attempts = 1;

        /**
         * A flag to include optional arguments during rendering function descriptions.
         */
        bool with_optional_arguments = false;
    };


    /**
     * Expects a `FunctionToolInvocation` message from context, and returns `FunctionToolResult` message.
     */
    class BaseFunctionTool: public virtual IFunctionTool, public BaseRunnable<FunctionToolInvocation, FunctionToolResult> {
        FunctionToolOptions options_;
    public:
        explicit BaseFunctionTool(const FunctionToolOptions &options)
            : options_(options) {
        }

        FunctionToolResult Invoke(const FunctionToolInvocation &invocation) override {
            return InvokeWithRetry_(invocation, 0);
        }

        virtual std::string Execute(const std::string& action_input) = 0;

        [[nodiscard]] const FunctionToolOptions& GetOptions() const {
            return options_;
        }

        FunctionToolSelfCheckResponse SelfCheck() override {
            FunctionToolSelfCheckResponse response;
            response.set_passed(true);
            return response;
        }

    private:
        FunctionToolResult InvokeWithRetry_(const FunctionToolInvocation &invocation, const uint8_t retry_count) {
            // if (retry_count > options_.max_attempts) {
            //     throw InstinctException(fmt::format("Abort function tool as max attempts reached. name={},id={}", invocation.name(), invocation.id()));
            // }
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
                if (retry_count+1 < options_.max_attempts) {
                    return InvokeWithRetry_(invocation, retry_count+1);
                }
                result.set_exception(e.what());
                LOG_ERROR("Finish function tool with exception: name={},id={},elapsed={}ms, ex.waht={}", GetSchema().name(), result.invocation_id(), ChronoUtils::GetCurrentTimeMillis() -  t1, e.what());
            }
            return result;
        }


    };

    using FunctionToolPtr = std::shared_ptr<BaseFunctionTool>;




}


#endif //BASEFUNCTIONTOOL_HPP
