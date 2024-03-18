//
// Created by RobinQu on 2024/3/11.
//

#ifndef MODELCALLBACKMIXINS_HPP
#define MODELCALLBACKMIXINS_HPP

#include "LLMGlobals.hpp"
#include "prompt/ChatPromptBuilder.hpp"

namespace INSTINCT_LLM_NS {

    using LLMPreGenerateCallback = std::function<void(const std::string& prompt)>;
    using LLMPostGenerateCallback = std::function<void(const std::string& prompt, const std::string& answer)>;

    class LLMCallbackMixins {
        std::vector<LLMPreGenerateCallback> pre_callbacks;
        std::vector<LLMPostGenerateCallback> post_callbacks;
    public:
        void RegisterPreGenerateCallback(const LLMPreGenerateCallback& callback) {
            pre_callbacks.push_back(callback);
        }

        void RegisterPostGenerateCallback(const LLMPostGenerateCallback& callback) {
            post_callbacks.push_back(callback);
        }


        template<typename R>
        requires RangeOf<R, std::string>
        void FirePreGenerateCallbacks(R&& range) {
            for (const auto& ele: range) {
                FirePreGenerateCallbacks(ele);
            }
        }

        void FirePreGenerateCallbacks(const std::string& prompt) const {
            // TODO move to another thread
            for (auto& callback: pre_callbacks) {
                try {
                    callback(prompt);
                } catch (const std::runtime_error& e) {
                    // TODO log error
                }
            }
        }

        template<typename R>
        requires RangeOf<R, std::string>
        void FirePostGenerateCallback(R&& prompts, R&& answer) {
            // for (const auto& ele: range) {
            //     FirePreGenerateCallbacks(ele);
            // }
        }

        void FirePostGenerateCallback(const std::string& prompt, const std::string& answer) const {
            // TODO move to another thread
            for (auto& callback: post_callbacks) {
                try {
                    callback(prompt, answer);
                } catch (const std::runtime_error& e) {
                    // TODO log error
                }
            }
        }
    };


    using ChatModelPreGenereateCallback = std::function<void(const MessageList& prompt)>;
    using ChatPostGenerateCallback = std::function<void(const MessageList& prompt, const Message& answer)>;
    class ChatModelCallbackMixins {
        std::vector<ChatModelPreGenereateCallback> pre_callbacks;
        std::vector<ChatPostGenerateCallback> post_callbacks;
    public:
        void RegisterPreGenerateCallback(const ChatModelPreGenereateCallback& callback) {
            pre_callbacks.push_back(callback);
        }

        void RegisterPostGenerateCallback(const ChatPostGenerateCallback& callback) {
            post_callbacks.push_back(callback);
        }

        void FirePreGenerateCallbacks(const MessageList& prompt) const {
            // TODO move to another thread
            for (auto& callback: pre_callbacks) {
                try {
                    callback(prompt);
                } catch (const std::runtime_error& e) {
                    // TODO log error
                }
            }
        }

        void FirePostGenerateCallback(const MessageList& prompt, const Message& answer) const {
            // TODO move to another thread
            for (auto& callback: post_callbacks) {
                try {
                    callback(prompt, answer);
                } catch (const std::runtime_error& e) {
                    // TODO log error
                }
            }
        }

    };
}


#endif //MODELCALLBACKMIXINS_HPP
