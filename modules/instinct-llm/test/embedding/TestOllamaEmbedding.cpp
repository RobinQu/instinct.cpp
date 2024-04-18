//
// Created by RobinQu on 2024/2/23.
//
#include <gtest/gtest.h>


#include "embedding_model/OllamaEmbedding.hpp"
#include "tools/TensorUtils.hpp"
#include <string>
#include <iostream>

#include "../document/Corpus.hpp"


namespace INSTINCT_LLM_NS {
    static std::vector<std::string> DOCS {
        R"""(Kazimierz Funk, commonly anglicized as Casimir Funk, was a Polish biochemist generally credited with being among the first to formulate the concept of vitamins, which he called "vital amines" or "vitamines")""",
        R"""(Albert Einstein was a German-born theoretical physicist who is widely held to be one of the greatest and most influential scientists of all time. )""",
        R"(The 1999 Sydney hailstorm was the costliest natural disaster in Australian history at the time as measured by insured damage. The storm developed south of Sydney, New South Wales, on the afternoon of 14 April 1999 and struck the city's central business district and its eastern suburbs later that evening. It dropped an estimated 500,000 tonnes of hailstones on Sydney and its suburbs. The insured damage bill was roughly A$1.7 billion, with the total bill (including uninsured damage) estimated to be around $2.3 billion. Lightning claimed one life, and the storm caused approximately 50 injuries. The storm was classified as a supercell following further analysis of its erratic nature and extreme attributes. The time of year and general conditions in the region were not seen as conducive for an extreme storm cell to form, and the Bureau of Meteorology was repeatedly surprised by its changes in direction, its duration, and the severity of the hail. )",
        R"(
Galena, also called lead glance, is the natural mineral form of lead(II) sulfide (PbS). In addition to lead, some deposits contain up to 0.5 percent silver, in the form of silver sulfide or as limited silver in solid solution; when present, this byproduct far surpasses the main lead ore in revenue. Galena has been used since antiquity, one of its oldest uses being the production of kohl, an eye cosmetic now regarded as toxic due to the risk of lead poisoning. In modern times, galena is primarily used to extract its constituent minerals. In addition to silver, it is the most important global source of lead, for uses such as in lead-acid batteries. This sample of galena, measuring 3.5 cm × 2.5 cm × 2.0 cm (1.38 in × 0.98 in × 0.79 in), contains a small amount of gold-colored pyrite and was extracted from the Huanzala Mine in the Peruvian region of Ancash. This photograph was focus-stacked from 156 separate images.)",
        R"(The Swift Orange Line is a bus rapid transit line in Snohomish County, Washington, United States. It is the third line of the Swift Bus Rapid Transit system operated by Community Transit and is scheduled to open in March 2024. The 11-mile (18 km) line runs from Edmonds College to McCollum Park, generally serving the cities of Lynnwood and Mill Creek with 16 stations. The Orange Line provides connections between the existing Swift Blue and Green lines, Edmonds College, Alderwood Mall, and Lynnwood Transit Center—the future terminus of the Link light rail system.)"
    };

    TEST(OllamaEmbedding, TestSimpleOps) {
        using namespace INSTINCT_CORE_NS;
        OllamaEmbedding embedding;
        TensorUtils::PrintEmbedding(embedding.EmbedQuery("hell word"));
        for(const auto& vector: embedding.EmbedDocuments(DOCS)) {
            TensorUtils::PrintEmbedding(vector);
        }
    }

    TEST(OllamaEmbedding, TestBatchExecute) {
        SetupLogging();
        for(OllamaEmbedding embedding({.max_paralle = 4}); const auto& vector: embedding.EmbedDocuments(DOCS)) {
            TensorUtils::PrintEmbedding(vector);
        }
    }



}
