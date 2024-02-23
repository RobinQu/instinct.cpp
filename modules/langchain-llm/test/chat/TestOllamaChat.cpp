//
// Created by RobinQu on 2024/2/23.
//
#include <gtest/gtest.h>

#include "chat_model/OllamaChat.hpp"

LC_LLM_NS {

    TEST(OllamaChat, SimpleTest) {
        OllamaChat ollama_chat;
        std::cout << std::visit(core::conv_message_to_string, ollama_chat.Invoke("Why sky is blue?", {})) << std::endl;
    }

}