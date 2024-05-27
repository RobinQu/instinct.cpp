//
// Created by RobinQu on 2024/3/26.
//

#ifndef PDFFILEINGESTOR_HPP
#define PDFFILEINGESTOR_HPP

#include <fpdfview.h>
#include <fpdf_text.h>
#include <unicode/unistr.h>
#include "tools/Assertions.hpp"
#include "BaseIngestor.hpp"
#include "RetrievalGlobals.hpp"
#include "tools/DocumentUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    class PDFIUMConfig {
    public:
        PDFIUMConfig() {
            FPDF_LIBRARY_CONFIG config;
            config.version = 2;
            config.m_pUserFontPaths = nullptr;
            config.m_pIsolate = nullptr;
            // config.m_pPlatform = nullptr;
            config.m_v8EmbedderSlot  = 0;
            FPDF_InitLibraryWithConfig(&config);
        }

        ~PDFIUMConfig() {
            FPDF_DestroyLibrary();
        }
    };


    class PDFIUMPage {
        FPDF_PAGE page_{};

    public:
        PDFIUMPage(FPDF_DOCUMENT doc, const int page_index)
            : page_(FPDF_LoadPage(doc, page_index)) {
        }

        explicit PDFIUMPage(FPDF_PAGE page)
            : page_(page) {
        }

        ~PDFIUMPage() {
            FPDF_ClosePage(page_);
        }

        UnicodeString ExtractPageText() {
            auto* text_page = FPDFText_LoadPage(page_);
            UnicodeString result;
            int buf_size = 1024;
            // int processed_count = 0;
            int start_index = 0;
            unsigned short buf[buf_size];
            const auto char_count = FPDFText_CountChars(text_page);
            while (true) {
                int processed_count = FPDFText_GetText(text_page, start_index, buf_size, buf);
                start_index += processed_count;
                result += UnicodeString(buf, processed_count);
                if(start_index >= char_count) {
                    break;
                }
            }
            FPDFText_ClosePage(text_page);
            return result;
        }
    };

    class PDFIUMDoc {
        FPDF_DOCUMENT doc_ = nullptr;

    public:
        PDFIUMDoc(const std::filesystem::path& file_path, const std::string& password) {
            doc_ = FPDF_LoadDocument(file_path.c_str(), password.c_str());
            if (!doc_) {
                unsigned long err = FPDF_GetLastError();
                auto path_string = file_path.string();
                std::string error_string;
                switch (err) {
                    case FPDF_ERR_SUCCESS:
                        error_string = fmt::format("Success loaded PDF at {}", path_string);
                        break;
                    case FPDF_ERR_UNKNOWN:
                        error_string = fmt::format("Unknown error while loading PDF at {}", path_string);
                        break;
                    case FPDF_ERR_FILE:
                        error_string = fmt::format("File not found or could not be opened: {}", path_string);
                        break;
                    case FPDF_ERR_FORMAT:
                        error_string = fmt::format("File not in PDF format or corrupted: {}", path_string);
                        break;
                    case FPDF_ERR_PASSWORD:
                        error_string = fmt::format("Password required or incorrect password: {}", path_string);
                        break;
                    case FPDF_ERR_SECURITY:
                        error_string = fmt::format("Unsupported security scheme: {}", path_string);
                        break;
                    case FPDF_ERR_PAGE:
                        error_string = fmt::format("Page not found or content error: {}", path_string);
                        break;
                    default:
                        error_string = fmt::format("Unknown error: code = {}", err);
                }
                throw InstinctException(error_string);
            }
        }

        ~PDFIUMDoc() {
            FPDF_CloseDocument(doc_);
        }

        [[nodiscard]] int GetPageCount() const {
            return FPDF_GetPageCount(doc_);
        }

        [[nodiscard]] PDFIUMPage LoadPage(const int i) const {
            return {doc_, i};
        }
    };

    static PDFIUMConfig GLOBAL_CONFIG;


    /**
    * Parsing PDF files using PDFium library
    */
    class PDFFileIngestor final : public BaseIngestor {
        std::filesystem::path file_path_{};
        const std::string password_;

    public:
        explicit PDFFileIngestor(std::filesystem::path file_path, std::string password = "")
            : file_path_(std::move(file_path)),
              password_(std::move(password)) {
            assert_true(std::filesystem::exists(file_path_), "Given filepath should be valid: " + file_path_.string());
        }

        AsyncIterator<Document> Load() override {
            return rpp::source::create<Document>([&](const auto& observer) {
                const auto pdf_doc = PDFIUMDoc(file_path_, password_);
                const int count = pdf_doc.GetPageCount();
                try {
                    for (int i = 0; i < count; i++) {
                        auto text = pdf_doc.LoadPage(i).ExtractPageText();
                        Document doc;
                        DocumentUtils::AddPresetMetadataFileds(doc, ROOT_DOC_ID, i+1, file_path_.string());
                        text.toUTF8String(*doc.mutable_text());
                        // auto* page_idx = doc.add_metadata();
                        // page_idx->set_name("page_no");
                        // page_idx->set_int_value(i+1);
                        // auto* source = doc.add_metadata();
                        // source->set_name("source");
                        // source->set_string_value(file_path_.string());
                        observer.on_next(doc);
                    }
                    observer.on_completed();
                } catch (...) {
                    observer.on_error(std::current_exception());
                }
            });
        }
    };

    static IngestorPtr CreatePDFFileIngestor(const std::filesystem::path& file_path, const std::string& password = "") {
        return std::make_shared<PDFFileIngestor>(file_path, password);
    }
}


#endif //PDFFILEINGESTOR_HPP
