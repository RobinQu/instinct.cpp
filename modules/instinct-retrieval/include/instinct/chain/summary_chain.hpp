//
// Created by RobinQu on 2024/6/3.
//

#ifndef SUMMARYCHAIN_HPP
#define SUMMARYCHAIN_HPP

#include <instinct/document/recursive_character_text_splitter.hpp>

#include <instinct/retrieval_global.hpp>
#include <instinct/chain/message_chain.hpp>
#include <instinct/chat_model/base_chat_model.hpp>
#include <instinct/output_parser/string_output_parser.hpp>
#include <instinct/prompt/plain_chat_prompt_template.hpp>
#include <instinct/prompt/plain_prompt_template.hpp>

namespace INSTINCT_RETRIEVAL_NS {

    namespace details {
        struct StreamBuffer {
            std::vector<std::string> data;
        };
    }

    template<typename R>
    requires RangeOf<R, std::string>
    class DocumentListInputParser final: public BaseInputParser<R> {
    public:
        explicit DocumentListInputParser(const InputParserOptions& options = {})
            : BaseInputParser<R>(options) {
        }

        JSONContextPtr ParseInput(const R &input) override {
            std::string buf;
            for(const std::string& text :input) {
                buf += text;
                buf += "\n";
            }
            return CreateJSONContext({ {"text", buf} });
        }

    };

    using SummaryChain = MessageChain<std::vector<std::string>, std::string>;
    using SummaryChainPtr = std::shared_ptr<SummaryChain>;

    /**
     * Create a
     * @param chat_model
     * @param prompt_template
     * @return
     */
    static SummaryChainPtr CreateSummaryChain(
        const ChatModelPtr& chat_model,
        PromptTemplatePtr prompt_template = nullptr
    ) {
        const auto input_parser = std::make_shared<DocumentListInputParser<std::vector<std::string>>>();
        const auto output_parser = std::make_shared<StringOutputParser>();
        if (!prompt_template) {
            prompt_template = CreatePlainPromptTemplate(R"(Please provide a concise summary of the following text in one sentence.
{text})", {.input_keys = {"text"}});
        }
        return CreateFunctionalChain<std::vector<std::string>, std::string>(
                input_parser,
                output_parser,
                prompt_template | chat_model->AsModelFunction()
            );
    }


    using IsReducibleFn = std::function<bool(const std::vector<std::string>& data, const std::string& delta)>;

    static IsReducibleFn CreateReducibleFnWithMaxStringSize(size_t limit) {
        return [limit] (const std::vector<std::string>& data, const std::string& delta) {
            size_t total=0;
            for(const auto& item:data) {
                total+=item.size();
            }
            return total+delta.size()>=limit;
        };
    }

    /**
     * Create summary with document source in map-reduce manner
     * @param source
     * @param chain the summary chain that handles a list of text strings
     * @param is_reducible_fn a boolean supplier that determines if previous data will be reduced
     * @return
     */
    static std::future<std::string> CreateSummary(
        const AsyncIterator<std::string>& source,
        const SummaryChainPtr& chain,
        const IsReducibleFn& is_reducible_fn) {
        assert_true(is_reducible_fn, "should provide is_reducible_fn");
        assert_true(chain, "should provide valid summary chain");
        return std::async(std::launch::async, [source,is_reducible_fn,chain] {
            const auto map_itr = rpp::source::create<std::vector<std::string>>([&,source](const  rpp::dynamic_observer<std::vector<std::string>>& ob) {
                const auto buf = std::make_shared<details::StreamBuffer>();
                source
                | rpp::ops::map([chain](const std::string& text) {
                    return chain->Invoke({text});
                })
                | rpp::ops::subscribe([ob,buf,is_reducible_fn](const std::string& text) {
                        if (!buf->data.empty() && is_reducible_fn(buf->data, text)) {
                            ob.on_next(buf->data);
                            buf->data.clear();
                        }
                        buf->data.push_back(text);
                    },
                    [ob](const std::exception_ptr& err) { ob.on_error(err); },
                    [ob,buf]() {
                        if (!buf->data.empty()) {
                            ob.on_next(buf->data);
                        }
                        ob.on_completed();
                    }
                );
            })
            | rpp::ops::map([chain](const std::vector<std::string>& batch) {
                return chain->Invoke(batch);
            });
            std::vector<std::string> summaries;
            CollectVector(map_itr, summaries);
            if (summaries.size()==1) {
                LOG_DEBUG("Got final summary: {}", summaries[0]);
                return summaries[0];
            }
            assert_true(summaries.size() > 1, "should have multiple summaries to continue");
            LOG_DEBUG("Recursively call with summaries.size()={}", summaries.size());
                // recursively call
                return CreateSummary(rpp::source::from_iterable(summaries), chain, is_reducible_fn).get();

        });
    }

    static std::future<std::string> CreateSummary(
        const AsyncIterator<Document>& source,
        const SummaryChainPtr& chain,
        const IsReducibleFn& is_reducible_fn) {
        return  CreateSummary(source | rpp::ops::map([](const Document& doc) {return doc.text();}), chain, is_reducible_fn);
    }
}

#endif //SUMMARYCHAIN_HPP
