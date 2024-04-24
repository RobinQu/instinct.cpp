//
// Created by RobinQu on 2024/4/19.
//

#include "assistant/v2/endpoint/AssistantController.hpp"

int main() {
    using namespace INSTINCT_SERVER_NS;
    using namespace INSTINCT_ASSISTANT_NS::v2;

    AssistantFacade facade {};
    const auto assistant_controller = std::make_shared<AssistantController>(facade);
    HttpLibServer server;
    server.Use(assistant_controller);
    server.StartAndWait();
}