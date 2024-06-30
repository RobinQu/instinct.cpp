//
// Created by RobinQu on 2024/6/30.
//
#include <instinct/ranker/jina_reranker_model.hpp>
#include <gtest/gtest.h>

#include "instinct/tools/document_utils.hpp"

namespace INSTINCT_LLM_NS {
    class TestJinaReranker: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }
    };

    TEST_F(TestJinaReranker, TestRerankWithDocuments) {
        const auto ranking_model = CreateJinaRerankerModel();
        std::vector<Document> docs;
        const auto texts = {"Organic skincare for sensitive skin with aloe vera and chamomile.",
    "New makeup trends focus on bold colors and innovative techniques",
    "Bio-Hautpflege für empfindliche Haut mit Aloe Vera und Kamille",
    "Neue Make-up-Trends setzen auf kräftige Farben und innovative Techniken",
    "Cuidado de la piel orgánico para piel sensible con aloe vera y manzanilla",
    "Las nuevas tendencias de maquillaje se centran en colores vivos y técnicas innovadoras",
    "针对敏感肌专门设计的天然有机护肤产品",
    "新的化妆趋势注重鲜艳的颜色和创新的技巧",
    "敏感肌のために特別に設計された天然有機スキンケア製品",
    "新しいメイクのトレンドは鮮やかな色と革新的な技術に焦点を当てています"};
        for(const auto& text: texts) {
            Document doc;
            doc.set_text(text);
            docs.push_back(doc);
        }
        const auto result = ranking_model->RerankDocuments(docs, "Organic skincare products for sensitive skin", 3);
        DocumentUtils::PrintRerankResult(result);
    }
}
