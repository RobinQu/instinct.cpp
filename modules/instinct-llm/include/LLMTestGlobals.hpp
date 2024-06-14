//
// Created by RobinQu on 2024/3/15.
//

#ifndef LLMTESTGLOBALS_HPP
#define LLMTESTGLOBALS_HPP
#include <random>

#include "chat_model/BaseChatModel.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "embedding_model/OpenAIEmbedding.hpp"
#include "llm/BaseLLM.hpp"
#include "llm/OpenAILLM.hpp"
#include "toolkit/BaseSearchTool.hpp"
#include "toolkit/LocalToolkit.hpp"
#include "toolkit/builtin/LLMMath.hpp"
#include "toolkit/builtin/SerpAPI.hpp"
#include "tools/ChronoUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    static Embedding make_zero_vector(const size_t dim=128) {
        Embedding embedding (dim);
        for(size_t i=0;i<dim;i++) {
            embedding.push_back(0);
        }
        return embedding;
    }

    static Embedding make_random_vector(const size_t dim=128) {
        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<float> dis(0, 1.0);

        Embedding embedding;
        embedding.reserve(dim);
        for(size_t i=0;i<dim;i++) {
            embedding.push_back(dis(gen));
        }
        return embedding;
    }

    class PesudoLLM final: public BaseLLM {
    public:
        void Configure(const ModelOverrides &options) override {}

        void BindTools(const FunctionToolkitPtr &toolkit) override {
            throw InstinctException("Not implemented");
        }
    private:
        BatchedLangaugeModelResult Generate(const std::vector<std::string> &prompts) override {
            BatchedLangaugeModelResult batched;
            for(const auto& prompt: prompts) {
                auto* model_result = batched.add_generations();
                auto* gen = model_result->add_generations();
                gen->set_text("You are right!");
                gen->set_is_chunk(false);
                gen->mutable_message()->set_content("You are right!");
                gen->mutable_message()->set_role("assistant");
            }
            return batched;
        }

        AsyncIterator<LangaugeModelResult> StreamGenerate(const std::string &prompt) override {
            std::vector<LangaugeModelResult> outputs;
            int n = 4;
            while (n-->0) {
                LangaugeModelResult result;
                auto *gen = result.add_generations();
                gen->mutable_message()->set_content(std::to_string(n));
                gen->mutable_message()->set_role("assistant");
            }
            return rpp::source::from_iterable(outputs);
        }
    };

    class PesudoChatModel final: public BaseChatModel {
    public:
        void Configure(const ModelOverrides &options) override {}

        void BindTools(const FunctionToolkitPtr &toolkit) override {
            throw InstinctException("Not implemented");
        }

    private:
        BatchedLangaugeModelResult Generate(const std::vector<MessageList> &messages) override {
            BatchedLangaugeModelResult batched_model_result;
            for(const auto& message_list: messages) {
                auto* result = batched_model_result.add_generations();
                auto* gen =result->add_generations();
                gen->set_text("talking non-sense");
                auto* msg = gen->mutable_message();
                msg->set_content(R"(
talking non-sense
talking non-sense
talking non-sense

talking non-sense
)");
                msg->set_role("assistant");
            }
            return batched_model_result;
        }

        AsyncIterator<LangaugeModelResult> StreamGenerate(const MessageList &messages) override {
            LangaugeModelResult model_result;
            return rpp::source::just(model_result);
        }


    };


    class PesudoEmbeddings final: public IEmbeddingModel {
        std::unordered_map<std::string, Embedding> caches_ = {};
        size_t dim_;
    public:
        explicit PesudoEmbeddings(const size_t dim = 512)
                : dim_(dim) {
        }

        std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            std::vector<Embedding> result;
            for(const auto& text: texts) {
                if (!caches_.contains(text)) {
                    caches_.emplace(text, make_random_vector(dim_));
                }
                result.push_back(caches_.at(text));
            }
            return result;
        }

        [[nodiscard]] auto& get_caches()  const {
            return caches_;
        }

        void clear_caches() {
            caches_.clear();
        }

        size_t GetDimension() override {
            return dim_;
        }


        Embedding EmbedQuery(const std::string& text) override {
            if (!caches_.contains(text)) {
                caches_.emplace(text, make_random_vector(dim_));
            }
            return caches_.at(text);
        }
    };

    static ChatModelPtr create_pesudo_chat_model() {
        return std::make_shared<PesudoChatModel>();
    }

    static LLMPtr  create_pesudo_llm() {
        return std::make_shared<PesudoLLM>();
    }

    static std::shared_ptr<PesudoEmbeddings> create_pesudo_embedding_model(size_t dim = 512) {
        return std::make_shared<PesudoEmbeddings>(dim);
    }

    static OpenAIConfiguration DEFAULT_NITRO_SERVER_CONFIGURATION = {
        .endpoint = {.host = "localhost", .port = 3928},
        .model_name = "local-model",
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


    class MockSearchTool final: public BaseSearchTool {
        std::multimap<std::string, SearchToolResponseEntry> entries_;
    public:
        explicit MockSearchTool(const FunctionToolOptions &options = {})
            : BaseSearchTool(options) {
        }

        void AddResponse(const std::string& q, const std::string& content, std::string title = "", std::string link = "") {
            SearchToolResponseEntry entry;
            entry.set_title(title);
            entry.set_content(content);
            if (StringUtils::IsBlankString(link)) {
                link = "https://google.com/search?q=" + q;
            }
            if (StringUtils::IsBlankString(title)) {
                title = "Search result about " + q;
            }
            entry.set_url(link);
            entries_.emplace(q, entry);
        }

        SearchToolResponse DoExecute(const SearchToolRequest &input) override {
            SearchToolResponse response;
            if (entries_.contains(input.query())) {
                // exact match
                auto range = entries_.equal_range(input.query());
                for(auto i=range.first; i!=range.second; ++i) {
                    response.add_entries()->CopyFrom(i->second);
                }
            } else {
                // give random result
                std::random_device r;
                // Choose a random mean between 1 and 6
                std::default_random_engine e1(r());
                std::uniform_int_distribution<int> uniform_dist(0, entries_.size());
                auto itr = entries_.begin();
                for(int i=0;i<uniform_dist(e1);++i) {
                    ++itr;
                }
                response.add_entries()->CopyFrom(itr->second);
            }
            return response;
        }

        FunctionToolSelfCheckResponse SelfCheck() override {
            FunctionToolSelfCheckResponse response;
            response.set_passed(true);
            return response;
        }
    };


     class GetNightlyHotelPrice final: public BaseFunctionTool {
            FunctionTool schema_;

        public:
            explicit GetNightlyHotelPrice(const FunctionToolOptions &options = {})
                : BaseFunctionTool(options) {
                ProtobufUtils::Deserialize(R"(
{
    "name": "get_nightly_hotel_price",
    "description": "Get hotel room price for a given city",
    "parameters": {
        "type":"object",
        "properties": {
            "city": {
                "type": "string",
                "description": "The city to get flight prices for"
            }
        },
        "required" : ["city"]
    }
}
)", schema_);
            }

            [[nodiscard]] const FunctionTool & GetSchema() const override {
                return schema_;
            }

            std::string Execute(const std::string &action_input) override {
                static std::unordered_map<std::string, float> price_maps {
                    {"New York", 300},
                    {"Paris", 200},
                    {"Tokyo", 300}
                };
                const auto args = nlohmann::json::parse(action_input);
                const auto city = args.at("city").get<std::string>();
                nlohmann::json resp;
                resp["city"] = city;
                resp["hotel_price"] = price_maps.at(city);
                return resp.dump();
            }
        };

        class GetFlightPriceTool final: public BaseFunctionTool {
            FunctionTool schema_;

        public:
            explicit GetFlightPriceTool(const FunctionToolOptions &options = {})
                : BaseFunctionTool(options) {
                ProtobufUtils::Deserialize(R"(
{
    "name": "get_flight_price",
    "description": "Get flight price for a given city",
    "parameters": {
        "type":"object",
        "properties": {
            "city": {
                "type": "string",
                "description": "The city to get flight prices for"
            }
        },
        "required" : ["city"]
    }
}
)", schema_);
            }

            [[nodiscard]] const FunctionTool & GetSchema() const override {
                return schema_;
            }

            std::string Execute(const std::string &action_input) override {
                static std::unordered_map<std::string, float> price_maps {
                            {"New York", 450},
                            {"Paris", 750},
                            {"Tokyo", 1200}
                };
                const auto args = nlohmann::json::parse(action_input);
                const auto city = args.at("city").get<std::string>();
                nlohmann::json resp;
                resp["city"] = city;
                resp["flight_price"] = price_maps.at(city);
                return resp.dump();
            }
        };


    class BaseAgentTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            DoSetUp();
        }

        virtual void DoSetUp() {}

        FunctionToolPtr search_tool_ = CreateSerpAPI({
                .apikey = SystemUtils::GetEnv("SERP_API_KEY")
            });

        ChatModelPtr chat_model_ = CreateOpenAIChatModel({
                .temperature = 0
            });

        FunctionToolPtr calculator_tool_ = CreateLLMMath(chat_model_, {.max_attempts = 2});

        FunctionToolkitPtr toolkit_ = CreateLocalToolkit({ search_tool_, calculator_tool_ });

    };


}


#endif //LLMTESTGLOBALS_HPP
