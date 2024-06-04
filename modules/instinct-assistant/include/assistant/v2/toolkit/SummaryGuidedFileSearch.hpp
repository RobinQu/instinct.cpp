//
// Created by RobinQu on 2024/6/4.
//

#ifndef SUMMARYGUIDEDFILESEARCH_HPP
#define SUMMARYGUIDEDFILESEARCH_HPP

#include "AssistantGlobals.hpp"
#include "assistant/v2/tool/SimpleRetrieverOperator.hpp"
#include "retrieval/BaseRetriever.hpp"
#include "toolkit/BaseSearchTool.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_RETRIEVAL_NS;

    namespace details {
        static std::string create_description(const std::vector<VectorStoreFileObject>& vector_store_file_objects) {
            std::string highlights;
            for(int i=0;i<vector_store_file_objects.size();++i) {
                highlights += std::to_string(i+1);
                highlights += ". ";
                highlights += vector_store_file_objects[i].summary();
                highlights += " ";
            }
            return fmt::format("Use this tool for more context information if user question is relevant to the the knowledge base which has following highlights: {}.", highlights);
        }
    }

    struct SummaryGuidedFileSearchOptions: public FunctionToolOptions {
        int top_file_n = 10;
    };

    /**
     * This search tool will narrow down search space by ranking input query with file summaries.
     */
    class SummaryGuidedFileSearch final: public BaseSearchTool {
        using PSF = std::pair<std::string, float>;

        RankingModelPtr ranking_model_;
        RetrieverPtr retriever_;
        std::vector<VectorStoreFileObject> vector_store_file_objects_;
        SummaryGuidedFileSearchOptions options_;

    public:
        SummaryGuidedFileSearch(
            RankingModelPtr ranking_model,
            RetrieverPtr retriever,
            std::vector<VectorStoreFileObject> vector_store_file_objects,
            const SummaryGuidedFileSearchOptions &options
        )
            : BaseSearchTool(options, "FileSearch", details::create_description(vector_store_file_objects)),
              ranking_model_(std::move(ranking_model)),
              retriever_(std::move(retriever)),
              vector_store_file_objects_(std::move(vector_store_file_objects)),
              options_(options) {
            assert_non_empty_range(vector_store_file_objects_);
        }

    protected:
        SearchToolResponse DoExecute(const SearchToolRequest &input) override {
            // file-id to score
            std::vector<PSF> file_scores;
            for(const auto& file: vector_store_file_objects_) {
                auto score = ranking_model_->Invoke({
                    .doc = file.summary(),
                    .query = input.query()
                });
                file_scores.emplace_back(file.file_id(), score);
            }

            // sort by score and select top N file_id
            std::ranges::sort(file_scores, [](const PSF& a, const PSF& b) {
                return a.second > b.second;
            });
            LOG_DEBUG("file scores after ranking: {}, top_file_n={}",
                StringUtils::JoinWith(file_scores | std::views::transform([](const PSF& pair){ return fmt::format("{}={}", pair.first, pair.second); }), ","),
                options_.top_file_n
            );

            // search with given file id
            SearchRequest search_request;
            search_request.set_query(input.query());
            search_request.set_top_k(input.result_limit());
            const auto file_id_terms = search_request.mutable_metadata_filter()->mutable_terms();
            file_id_terms->set_name(METADATA_SCHEMA_PARENT_DOC_ID_KEY);
            for(int i=0;i<options_.top_file_n && i<file_scores.size();++i) {
                file_id_terms->add_terms()->set_string_value(file_scores.at(i).first);
            }
            SearchToolResponse search_tool_response;
            retriever_->Retrieve(search_request)
                | rpp::ops::as_blocking()
                | rpp::ops::subscribe([&](const Document& doc) {
                    auto* entry = search_tool_response.add_entries();
                    // TODO more data fields in metadata
                    entry->set_title(doc.id());
                    entry->set_content(doc.text());
                });
            return search_tool_response;
        }
    };

    static FunctionToolPtr CreateSummaryGuidedFileSearch(
            const RankingModelPtr& ranking_model,
            const RetrieverPtr& retriever,
            const std::vector<VectorStoreFileObject>& vector_store_file_objects,
            const SummaryGuidedFileSearchOptions &options = {}) {
        return std::make_shared<SummaryGuidedFileSearch>(ranking_model, retriever, vector_store_file_objects, options);
    }

}

#endif //SUMMARYGUIDEDFILESEARCH_HPP
