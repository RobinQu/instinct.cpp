//
// Created by RobinQu on 2024/4/6.
//

#ifndef AGENTGLOBALS_HPP
#define AGENTGLOBALS_HPP


#define INSTINCT_ASSISTANT_NS instinct::assistant


#include <assistant_api_v2.pb.h>
#include <string>
#include <inja/inja.hpp>

#include <instinct/core_global.hpp>
#include <instinct/data_global.hpp>
#include <instinct/chain/message_chain.hpp>
#include <instinct/toolkit/function_tool.hpp>
#include <instinct/tools/snowflake_id_generator.hpp>
#include <instinct/llm_global.hpp>

namespace INSTINCT_ASSISTANT_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    namespace v2 {
        static const std::string FILE_SEARCH_TOOL_NAME = "FileSearch";

        static std::string to_string(const ListRequestOrder order) {
            if (order == asc) return "asc";
            if (order == desc) return "desc";
            return "";
        }

        static std::string to_string(const FileObjectPurpose purpose) {
            if (purpose == assistants) {
                return "assistants";
            }
            if (purpose == assistants_output) {
                return "assistants_output";
            }
            return "unknown";
        }

        namespace details {
            static std::string map_file_object_key(FileObjectPurpose purpose, const std::string& file_id) {
                const auto partition_id = file_id.back() % 10;
                return fmt::format("{}/{}/{}", to_string(purpose), partition_id, file_id);
            }
            using INSTINCT_LLM_NS::details::generate_next_object_id;

        }

        static constexpr int DEFAULT_LIST_LIMIT = 20;

    }


}

#endif //AGENTGLOBALS_HPP
