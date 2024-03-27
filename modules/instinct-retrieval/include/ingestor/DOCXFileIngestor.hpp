//
// Created by RobinQu on 2024/3/26.
//

#ifndef DOCXFILEINGESTOR_HPP
#define DOCXFILEINGESTOR_HPP

#include <duckx.hpp>

#include "BaseIngestor.hpp"
#include "tools/DocumentUtils.hpp"
#include "tools/Assertions.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    /**
     * Parsing documents using DuckX library.
     * https://github.com/amiremohamadi/DuckX/?tab=readme-ov-file
     *
     * Currently this library is lacking unicoode support.
     *
     */
    class DOCXFileIngestor final: public BaseIngestor {
        std::filesystem::path file_path_{};

    public:
        explicit DOCXFileIngestor(std::filesystem::path file_path)
            : file_path_(std::move(file_path)) {
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
        Document ProduceDoc_(const std::string& text, const int idx) {
            Document document;
            document.set_text(text);
            DocumentUtils::AddPresetMetadataFileds(document, ROOT_DOC_ID, idx, file_path_.string());
            return document;
        }
    };


    static IngestorPtr CreateDOCXFileIngestor(const std::filesystem::path& file_path) {
        return std::make_shared<DOCXFileIngestor>(file_path);
    }



}


#endif //DOCXFILEINGESTOR_HPP
