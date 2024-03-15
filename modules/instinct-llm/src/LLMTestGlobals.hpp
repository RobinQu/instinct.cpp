//
// Created by RobinQu on 2024/3/15.
//

#ifndef LLMTESTGLOBALS_HPP
#define LLMTESTGLOBALS_HPP


#include "RetrievalGlobals.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "embedding_model/OpenAIEmbedding.hpp"
#include "llm/BaseLLM.hpp"
#include "llm/OpenAILLM.hpp"
#include "tools/ChronoUtils.hpp"

namespace INSTINCT_LLM_NS::test {
    using namespace INSTINCT_CORE_NS;

    static OpenAIConfiguration DEFAULT_NITRO_SERVER_CONFIGURATION = {
        .model_name = "local-model",
        .endpoint = {.host = "localhost", .port = 3928},
        .dimension = 512
    };

    static std::filesystem::path ensure_random_temp_folder() {
        auto root_path = std::filesystem::temp_directory_path() / "instinct-test" / std::to_string(
                             ChronoUtils::GetCurrentTimeMillis());
        std::filesystem::create_directories(root_path);
        return root_path;
    }

    static ChatModelPtr create_local_chat_model() {
        return CreateOpenAIChatModel(DEFAULT_NITRO_SERVER_CONFIGURATION);
    }

    static LLMPtr create_local_llm() {
        return CreateOpenAILLM(DEFAULT_NITRO_SERVER_CONFIGURATION);
    }

    static EmbeddingsPtr create_local_embedding_model(const size_t dimension = 512) {
        auto conf = DEFAULT_NITRO_SERVER_CONFIGURATION;
        conf.dimension = dimension;
        return CreateOpenAIEmbeddingModel(conf);
    }
}


#endif //LLMTESTGLOBALS_HPP
