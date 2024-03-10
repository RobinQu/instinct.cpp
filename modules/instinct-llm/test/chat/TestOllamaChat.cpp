//
// Created by RobinQu on 2024/2/23.
//
#include <gtest/gtest.h>

#include "chat_model/OllamaChat.hpp"
#include "prompt/ChatPromptBuilder.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class OllamaChatTest: public testing::Test {
    protected:
        void SetUp() override {
            dialog1 = MessageUtils::CreateOllamaChatPromptBuilder();
            dialog2 = MessageUtils::CreateOllamaChatPromptBuilder();

            dialog1->AddHumanMessage("what's product of 2 and 4?")
            ->AddAIMessage("The product of 2 and 4 is 8.")
            ->AddHumanMessage("what's remianing of 8 divided by 3 ?")
            ->AddAIMessage("The remainder of 8 divided by 3 is 1.")
            ->AddHumanMessage("The remainder of 8 divided by 3 should be 2. Do your concur?");

            dialog2
            ->AddHumanMessage("why sky is blue?")
            ->AddAIMessage(R""(The sky appears blue because of a phenomenon called Rayleigh scattering. When sunlight enters Earth's atmosphere, it encounters tiny molecules of gases such as nitrogen and oxygen.
These molecules scatter the light in all directions, but they scatter shorter (blue) wavelengths more than longer (red) wavelengths. This is known as Rayleigh scattering.

As a result of this scattering, the blue light is dispersed throughout the atmosphere, giving the sky its blue color. The red light, on the other hand, passes through the atmosphere
mostly unscattered, which is why we see the sun as yellow or orange during sunrise and sunset.

Other factors can also affect the color of the sky, such as air pollution, dust, and water vapor. However, in general, the blue color of the sky is due to Rayleigh scattering.

It's worth noting that the exact shade of blue can vary depending on the time of day, the amount of sunlight present, and other environmental factors. For example, during sunrise and
sunset, the sky may take on a more orange or red hue due to the angle of the sunlight and the scattering of light by atmospheric particles.)"")
            ->AddHumanMessage("can you explain in one sentence?")
            ->AddAIMessage("The sky appears blue because of Rayleigh scattering, where shorter (blue) wavelengths of sunlight are scattered throughout the atmosphere more than longer (red) wavelengths, giving the sky its blue color.")
            ->AddAIMessage(R"""(The sky is blue because it's like a big ol' blanket of air up there! And when the sun shines through that air, it's like it's playing hide-and-seek with the light and scattering the
blue bits all around so we can see them!)""")
            ->AddHumanMessage("Please explain why sea is blue in the same way.")
            ;
        }
        ChatPromptBuliderPtr dialog1;
        ChatPromptBuliderPtr dialog2;
    };

    TEST_F(OllamaChatTest, SimpleTest) {
        OllamaChat ollama_chat;
        auto result = ollama_chat.Invoke("Why sky is blue?");
        std::cout << result.DebugString() << std::endl;
    }

    TEST_F(OllamaChatTest, TestBatch) {
        OllamaChat ollama_chat;
        auto result = ollama_chat.Batch({
            dialog1->Build(),
            dialog2->Build()
        });
        while (result->HasNext()) {
            std::cout << result->Next().DebugString() << std::endl;
        }
    }

    TEST_F(OllamaChatTest, TestStream) {
        OllamaChat ollama_chat;
        auto result = ollama_chat.Stream(dialog1->Build());
        while (result->HasNext()) {
            std::cout << result->Next().DebugString() << std::endl;
        }
    }
}
