//
// Created by RobinQu on 2024/2/23.
//
#include <gtest/gtest.h>

#include "chat_model/OllamaChat.hpp"

LC_LLM_NS {
    using namespace langchain::core;

    static MessageVariants dialog1 = {
        HumanMessage{"what's product of 2 and 4"},
        AIMessage{"The product of 2 and 4 is 8."},
        HumanMessage{"what's remianing of 8 divided by 3 ?"},
        AIMessage{"The remainder of 8 divided by 3 is 1."},
        HumanMessage{"The remainder of 8 divided by 3 should be 2. Do your concur?"}
    };

    static MessageVariants dialog2 = {
        HumanMessage("why sky is blue?"),
        AIMessage{
            R"""(The sky appears blue because of a phenomenon called Rayleigh scattering. When sunlight enters Earth's atmosphere, it encounters tiny molecules of gases such as nitrogen and oxygen.
These molecules scatter the light in all directions, but they scatter shorter (blue) wavelengths more than longer (red) wavelengths. This is known as Rayleigh scattering.

As a result of this scattering, the blue light is dispersed throughout the atmosphere, giving the sky its blue color. The red light, on the other hand, passes through the atmosphere
mostly unscattered, which is why we see the sun as yellow or orange during sunrise and sunset.

Other factors can also affect the color of the sky, such as air pollution, dust, and water vapor. However, in general, the blue color of the sky is due to Rayleigh scattering.

It's worth noting that the exact shade of blue can vary depending on the time of day, the amount of sunlight present, and other environmental factors. For example, during sunrise and
sunset, the sky may take on a more orange or red hue due to the angle of the sunlight and the scattering of light by atmospheric particles.)"""
        },
        HumanMessage{"can you explain in one sentence?"},
        AIMessage{
            "The sky appears blue because of Rayleigh scattering, where shorter (blue) wavelengths of sunlight are scattered throughout the atmosphere more than longer (red) wavelengths, giving the sky its blue color."
        },
        HumanMessage{"can you explain in one sentence and in a easy to understand way for a child ?"},
        AIMessage(
            R"""(The sky is blue because it's like a big ol' blanket of air up there! And when the sun shines through that air, it's like it's playing hide-and-seek with the light and scattering the
blue bits all around so we can see them!)"""),
        HumanMessage {"Please explain why sea is blue in the same way."}
    };


    static std::vector<core::LanguageModelInput> dialogs = {
        dialog1,
        dialog2
    };

    TEST(OllamaChat, SimpleTest) {
        OllamaChat ollama_chat;
        std::cout << std::visit(core::conv_message_to_string, ollama_chat.Invoke("Why sky is blue?")) << std::endl;
    }

    TEST(OllamaChat, TestBatch) {
        OllamaChat ollama_chat;
        auto* result = ollama_chat.Batch(dialogs);
        while (result->HasNext()) {
            std::cout << std::visit(core::conv_message_to_string, result->Next()) << std::endl;
        }
        delete result;
    }

    TEST(OllamaChat, TestStream) {
        OllamaChat ollama_chat;
        auto* result = ollama_chat.Stream(dialog1);
        while (result->HasNext()) {
            std::cout << std::visit(core::conv_message_to_string, result->Next()) << std::endl;
        }
    }
}
