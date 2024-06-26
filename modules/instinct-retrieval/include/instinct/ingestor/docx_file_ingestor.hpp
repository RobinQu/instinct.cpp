//
// Created by RobinQu on 2024/3/26.
//

#ifndef DOCXFILEINGESTOR_HPP
#define DOCXFILEINGESTOR_HPP

#include <duckx.hpp>

#include <instinct/ingestor/ingestor.hpp>
#include <instinct/tools/document_utils.hpp>
#include <instinct/tools/assertions.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    /**
     * Parsing documents using DuckX library.
     * https://github.com/amiremohamadi/DuckX/?tab=readme-ov-file
     *
     * Currently this library is lacking unicoode support.
     *
     */
    class DOCXFileIngestor final: public BaseIngestor {
        std::filesystem::path file_path_;
        std::string file_source_;

    public:
        explicit DOCXFileIngestor(std::filesystem::path file_path, const DocumentPostProcessor &document_post_processor = nullptr, std::string file_source = "")
            : BaseIngestor(document_post_processor),
            file_path_(std::move(file_path)), file_source_(std::move(file_source)) {
            assert_true(std::filesystem::exists(file_path_), "Given file path should be valid: " + file_path_.string());
        }

        AsyncIterator<Document> Load() override {
            return rpp::source::create<Document>([&](const auto& observer) {
                static int max_paragraph_per_doc = 20;

                duckx::Document docx {file_path_.string()};
                docx.open();
                try {
                    int i=0, page_no = 0;
                    std::string buf;
                    for(auto p = docx.paragraphs(); p.has_next(); p.next()) {
                        for(auto r = p.runs(); r.has_next(); r.next()) {
                            buf += r.get_text();
                        }
                        if (++i % max_paragraph_per_doc == 0) {
                            if(!StringUtils::IsBlankString(buf)) {
                                observer.on_next(ProduceDoc_(buf, ++page_no));
                            }
                            buf.clear();
                        }
                    }
                    if (!buf.empty()) {
                        if(!StringUtils::IsBlankString(buf)) {
                            observer.on_next(ProduceDoc_(buf, ++page_no));
                        }
                        buf.clear();
                    }
                    observer.on_completed();
                } catch (...) {
                    observer.on_error(std::current_exception());
                }
            });
        }

    private:
        [[nodiscard]] Document ProduceDoc_(const std::string& text, const int idx) const {
            return CreateNewDocument(text, ROOT_DOC_ID, idx,
            StringUtils::IsBlankString(file_source_) ? file_path_.string() : file_source_
            );
        }
    };


    static IngestorPtr CreateDOCXFileIngestor(const std::filesystem::path& file_path, const DocumentPostProcessor& document_post_processor = nullptr, const std::string& source_id = "") {
        return std::make_shared<DOCXFileIngestor>(file_path, document_post_processor, source_id);
    }



}


#endif //DOCXFILEINGESTOR_HPP
