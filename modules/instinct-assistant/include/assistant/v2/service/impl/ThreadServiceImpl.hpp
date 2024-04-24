//
// Created by RobinQu on 2024/4/24.
//

#ifndef THREADSERVICEIMPL_HPP
#define THREADSERVICEIMPL_HPP
#include "AssistantTestGlobals.hpp"
#include "assistant/v2/service/IMessageService.hpp"
#include "assistant/v2/service/IThreadService.hpp"


namespace INSTINCT_ASSISTANT_NS {
    class ThreadServiceImpl final: public IThreadService {
        DataMapperPtr<ThreadObject, std::string> thread_data_mapper_;
        DataMapperPtr<MessageObject, std::string> message_data_mapper_;
    public:
        std::optional<ThreadObject> CreateThread(const ThreadObject &create_request) override {
            // TODO with transaction
            for(const auto& msg: create_request.messages()) {
                CreateMessage_(msg);
            }
            auto id = CreateThraed_(create_request);
            GetThreadRequest get_thread_request;
            get_thread_request.set_thread_id(id);
            return RetrieveThread(get_thread_request);
        }

        std::optional<ThreadObject> RetrieveThread(const GetThreadRequest &retrieve_request) override {
            assert_not_blank(retrieve_request.thread_id());
            return thread_data_mapper_->SelectOne("select * from instinct_thread where id = {{text(id)}};", {
                {"id", retrieve_request.thread_id()}
            });
        }

        std::optional<ThreadObject> ModifyThread(const ModifyThreadRequest &modify_request) override {

        }

        DeleteThreadResponse DeleteThread(const DeleteThreadResponse &delete_request) override {
            assert_not_blank(delete_request.id());
            const auto count = thread_data_mapper_->Execute("delete from instinct_thread where id = {{text(id)}};", {{"id", delete_request.id()}});
            DeleteThreadResponse delete_thread_response;
            delete_thread_response.set_deleted(count == 1);
            delete_thread_response.set_object("thread.deleted");
            delete_thread_response.set_id(delete_request.id());
            return delete_thread_response;
        }

    private:
        void CreateMessage_(const MessageObject& message) {

        }

        std::string CreateThraed_(const ThreadObject& thread) {

        }
    };
}

#endif //THREADSERVICEIMPL_HPP
