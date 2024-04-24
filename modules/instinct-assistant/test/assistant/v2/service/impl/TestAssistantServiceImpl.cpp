//
// Created by RobinQu on 2024/4/23.
//
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include "AssistantTestGlobals.hpp"


namespace INSTINCT_ASSISTANT_NS {

    class AssistantServiceTest: public BaseAssistantApiTest {
    public:
        AssistantServicePtr CreateService() {
            return std::make_shared<AssistantServiceImpl>(assistant_data_mapper);
        }
    };

    TEST_F(AssistantServiceTest, SimpleCRUD) {
        auto assistant_service = CreateService();

        AssistantObject create_request;
        create_request.set_model("ollama/mixtral:latest");
        auto object = assistant_service->CreateAssistant(create_request);
        ASSERT_EQ(object->model(), "ollama/mixtral:latest");

        GetAssistantRequest get_assistant_request;
        get_assistant_request.set_assistant_id(object->id());
        auto object2 = assistant_service->RetrieveAssistant(get_assistant_request);

        auto diff = util::MessageDifferencer {};
        ASSERT_TRUE(diff.Compare(object.value(), object2.value()));
    }

}