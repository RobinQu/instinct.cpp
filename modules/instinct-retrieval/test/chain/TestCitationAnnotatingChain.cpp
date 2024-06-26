//
// Created by RobinQu on 2024/6/9.
//
#include <gtest/gtest.h>
#include <instinct/chain/CitationAnnotatingChain.hpp>
#include <instinct/chat_model/openai_chat.hpp>


namespace INSTINCT_RETRIEVAL_NS {
    class TestCitationAnnotatingChain: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }
    };


    TEST_F(TestCitationAnnotatingChain, TestAnnotation) {
        CitationAnnotatingContext citation_annotating_context;
        const std::vector<std::string> context = {
            "Adults weigh between 21 and 72 kg (46 and 159 lb). The cheetah is capable of running at 93 to 104 km/h (58 to 65 mph); it has evolved specialized adaptations for speed, including a light build, long thin legs and a long tail",
            "The cheetah (Acinonyx jubatus) is a large cat and the fastest land animal. It has a tawny to creamy white or pale buff fur that is marked with evenly spaced, solid black spots. The head is small and rounded, with a short snout and black tear-like facial streaks. It reaches 67–94 cm (26–37 in) at the shoulder, and the head-and-body length is between 1.1 and 1.5 m (3 ft 7 in and 4 ft 11 in)",
            "The cheetah, the fastest land mammal, scores at only 16 body lengths per second, while Anna's hummingbird has the highest known length-specific velocity attained by any vertebrate",
            "It feeds on small- to medium-sized prey, mostly weighing under 40 kg (88 lb), and prefers medium-sized ungulates such as impala, springbok and Thomson's gazelles. The cheetah typically stalks its prey within 60–100 m (200–330 ft) before charging towards it, trips it during the chase and bites its throat to suffocate it to death. It breeds throughout the year",
        "The cheetah was first described in the late 18th century. Four subspecies are recognised today that are native to Africa and central Iran. An African subspecies was introduced to India in 2022. It is now distributed mainly in small, fragmented populations in northwestern, eastern and southern Africa and central Iran",
        R"(The cheetah lives in three main social groups: females and their cubs, male "coalitions", and solitary males. While females lead a nomadic life searching for prey in large home ranges, males are more sedentary and instead establish much smaller territories in areas with plentiful prey and access to females. The cheetah is active during the day, with peaks during dawn and dusk)",
        "The Southeast African cheetah (Acinonyx jubatus jubatus) is the nominate cheetah subspecies native to East and Southern Africa. The Southern African cheetah lives mainly in the lowland areas and deserts of the Kalahari, the savannahs of Okavango Delta, and the grasslands of the Transvaal region in South Africa. In Namibia, cheetahs are mostly found in farmlands",
        R"(Subpopulations have been called "South African cheetah" and "Namibian cheetah.")",
        "In India, four cheetahs of the subspecies are living in Kuno National Park in Madhya Pradesh after having been introduced there",
        "Acinonyx jubatus velox proposed in 1913 by Edmund Heller on basis of a cheetah that was shot by Kermit Roosevelt in June 1909 in the Kenyan highlands. Acinonyx rex proposed in 1927 by Reginald Innes Pocock on basis of a specimen from the Umvukwe Range in Rhodesia."
        };
        for(int i=0;const auto& txt: context) {
            i++;
            auto* entry = citation_annotating_context.mutable_original_search_response()->add_entries();
            entry->set_parent_doc_id("doc_" + std::to_string(i));
            entry->set_content(txt);
            entry->set_start_index(i*100);
            entry->set_end_index(i*200);
        }
        citation_annotating_context.set_question("How fast are cheetahs?");
        citation_annotating_context.set_original_answer("Cheetahs are the fastest land animals, capable of running at speeds between 93 to 104 kilometers per hour (58 to 65 miles per hour). However, they only score at 16 body lengths per second, which is lower than Anna's hummingbird's length-specific velocity.");
        const auto chat_model = CreateOpenAIChatModel();
        const auto chain = CreateCitationAnnotatingChain(chat_model);
        const AnswerWithCitations answer_with_annotation = chain->Invoke(citation_annotating_context);
        LOG_INFO("answer_with_annotation={}", answer_with_annotation.ShortDebugString());
        ASSERT_TRUE(StringUtils::IsNotBlankString(answer_with_annotation.answer()));
        ASSERT_GT(answer_with_annotation.citations_size(), 0);
        for(const auto& citation: answer_with_annotation.citations()) {
            ASSERT_TRUE(StringUtils::IsNotBlankString(citation.annotation()));
            ASSERT_TRUE(citation.quoted_index()>0);
        }
    };
}
