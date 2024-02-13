//
// Created by RobinQu on 2024/2/12.
//

#ifndef OLLAMACHAT_H
#define OLLAMACHAT_H
#include "model/BaseChatModel.h"
#include "tools/HttpRestClient.h"
#include "ModelGlobals.h"

namespace LC_MODEL_NS {

class OllamaChat: public core::BaseChatModel {
    std::string model_name_;
    core::HttpRestClient client_;
public:
    OllamaChat();
    explicit OllamaChat(core::Endpoint endpoint);
protected:
    core::ChatResultPtr Generate(const std::vector<std::vector<core::BaseMessagePtr>>& messages,
                                 const std::vector<std::string>& stop_words, const core::OptionDict& options) override;
};

} // core
// langchain

#endif //OLLAMACHAT_H
