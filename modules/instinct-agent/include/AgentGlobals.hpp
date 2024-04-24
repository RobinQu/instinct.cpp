//
// Created by RobinQu on 2024/4/6.
//

#ifndef AGENTGLOBALS_HPP
#define AGENTGLOBALS_HPP


#define INSTINCT_AGENT_NS instinct::agent
#include <assistant_api_v2.pb.h>
#include <string>
#include <inja/inja.hpp>

#include "CoreGlobals.hpp"
#include "chain/MessageChain.hpp"
#include "toolkit/BaseFunctionTool.hpp"
#include "tools/SnowflakeIDGenerator.hpp"

namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;


    namespace assistant::v2 {
        namespace details {
            static std::string generate_next_object_id(const std::string_view& prefix) {
                static SnowflakeIDGenerator<1534832906275L> generator;
                return fmt::format("{}-{}", prefix, generator.NextID());
            }

        }

        static constexpr int DEFAULT_LIST_LIMIT = 20;


        static std::string to_string(const ListRequestOrder order) {
            if (order == asc) return "asc";
            if (order == desc) return "desc";
            return "";
        }



    }


}



#endif //AGENTGLOBALS_HPP
