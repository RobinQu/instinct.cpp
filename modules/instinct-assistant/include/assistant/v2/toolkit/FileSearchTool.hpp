//
// Created by RobinQu on 2024/5/28.
//

#ifndef FILESEARCHTOOL_HPP
#define FILESEARCHTOOL_HPP

#include "toolkit/ProtoMessageFunctionTool.hpp"
#include "AssistantGlobals.hpp"
#include "retrieval/BaseRetriever.hpp"
#include "assistant/v2/tool/SimpleRetrieverOperator.hpp"
#include "toolkit/BaseSearchTool.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_RETRIEVAL_NS;
    struct FileSearchToolOptions: public FunctionToolOptions {

    };

    class FileSearchTool final: public BaseSearchTool {
        RetrieverPtr retriever_;

    public:
        FileSearchTool(const std::string &name, const std::string &description,
            RetrieverPtr retriever, const FunctionToolOptions &options = {})
            : BaseSearchTool(options, name, description),
              retriever_(std::move(retriever)) {
        }

    protected:
        SearchToolResponse DoExecute(const SearchToolRequest &input) override {
            const auto doc_itr = retriever_->Retrieve({.text = input.query(), .top_k = input.result_limit()});
            SearchToolResponse response;
            doc_itr
                | rpp::ops::as_blocking()
                | rpp::ops::subscribe([&](const Document& doc) {
                    auto* entry = response.mutable_entries()->Add();
                    entry->set_title(doc.id());
                    entry->set_content(doc.text());
                    return entry;
                });
            return response;
        }
    };
}

#endif //FILESEARCHTOOL_HPP
