//
// Created by RobinQu on 2024/4/25.
//
#include <gtest/gtest.h>
#include "AssistantTestGlobals.hpp"
#include "assistant/v2/service/IThreadService.hpp"
#include "assistant/v2/service/impl/ThreadServiceImpl.hpp"

namespace INSTINCT_ASSISTANT_NS {
    class ThreadServiceTest: public BaseAssistantApiTest {
    public:
        ThreadServicePtr CreateService() {
            return std::make_shared<ThreadServiceImpl>(thread_data_mapper, message_data_mapper);
        }
    };

    TEST_F(ThreadServiceTest, SimpleCRUD) {
        const auto thread_service = CreateService();
        ThreadObject create_request1;
        create_request1.mutable_tool_resources()->mutable_file_search()->add_vector_store_ids("vs-1234");
        thread_service->CreateThread(create_request1);

    }
}
