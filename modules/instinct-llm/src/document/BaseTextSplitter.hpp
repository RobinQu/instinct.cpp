//
// Created by RobinQu on 2024/3/5.
//

#ifndef BASETEXTSPLITTER_HPP
#define BASETEXTSPLITTER_HPP

#include <retrieval.pb.h>

#include "TextSplitter.hpp"
#include "tools/ResultIterator.hpp"


namespace INSTINCT_LLM_NS {
    using namespace U_ICU_NAMESPACE;

    using LengthFunction = std::function<size_t(const UnicodeString&)> ;

    static LengthFunction IdentityLengthFunction = [](const UnicodeString& s) {
        return s.length();
    };

    static std::string CHUNK_DOC_PART_INDEX_KEY = "chunk_id";

    class BaseTextSplitter: public TextSplitter {
    protected:
        int chunk_size_;
        int chunk_overlap_;
        bool keep_sepeartor_;
        bool strip_whitespace_;
        LengthFunction length_function_;
    public:
        BaseTextSplitter();

        BaseTextSplitter(const int chunk_size, const int chunk_overlap, const bool keep_sepeartor,
            const bool strip_whitespace, LengthFunction length_function)
            : chunk_size_(chunk_size),
              chunk_overlap_(chunk_overlap),
              keep_sepeartor_(keep_sepeartor),
              strip_whitespace_(strip_whitespace),
              length_function_(std::move(length_function)) {
        }

        ResultIteratorPtr<Document> SplitDocuments(const ResultIteratorPtr<Document>& docs_itr) override {
            std::vector<Document> chunked_docs;
            while (docs_itr->HasNext()) {
                auto& doc = docs_itr->Next();
                auto chunks = SplitText(UnicodeString::fromUTF8(doc.text()));
                for (int i=0;i<chunks.size();i++) {
                    auto& chunk = chunks[i];
                    Document document;
                    chunk.toUTF8String(*document.mutable_text());
                    chunked_docs.push_back(document);
                    auto* chunk_id_field = document.add_metadata();
                    chunk_id_field->set_name(CHUNK_DOC_PART_INDEX_KEY);
                    chunk_id_field->set_int_value(i);
                }
            }
            return create_result_itr_from_range(chunked_docs);
        }

    protected:

        void MergeSplits_(const std::vector<UnicodeString>& splits, const UnicodeString& seperator, std::vector<UnicodeString>& docs) const {
            const auto s_len = length_function_(seperator);
            // std::vector<UnicodeString> docs;
            std::vector<UnicodeString> current_doc;
            size_t total = 0;
            for(const auto& s: splits) {
                const auto d_len = length_function_(s);
                if (total + d_len + (current_doc.empty() ? 0: s_len) > chunk_size_) {
                    if(!current_doc.empty()) {
                        if(const auto doc = JoinDocs_(current_doc, seperator); doc.length()) {
                            docs.push_back(doc);
                        }
                        if (chunk_overlap_!=0) {
                            // handle overlapping
                            while(total > chunk_overlap_ && !current_doc.empty()) {
                                // strip first item until remianing chunks are enough for overlapping
                                total -= length_function_(current_doc.front()) + (current_doc.empty() ? 0: s_len);
                                current_doc.erase(current_doc.begin());
                            }
                        } else {
                            total = 0;
                            current_doc.clear();
                        }
                    }
                }
                total += d_len + (current_doc.empty() ? 0 : s_len);
                current_doc.push_back(s);
            }
            if (const auto rest = JoinDocs_(current_doc, seperator); rest.length()) {
                docs.push_back(rest);
            }
        }

        [[nodiscard]] UnicodeString JoinDocs_(const std::vector<UnicodeString>& docs, const UnicodeString& seperator) const  {
            UnicodeString text;
            for(int i=0; i<docs.size(); i++) {
                text+=docs[i];
                if(i+1 < docs.size()) {
                    text+=seperator;
                }
            }
            if(strip_whitespace_) {
                return text.trim();
            }
            return text;
        }

    };

}

#endif //BASETEXTSPLITTER_HPP
