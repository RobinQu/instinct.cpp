//
// Created by RobinQu on 2024/3/26.
//

#ifndef HTTPLIBSERVER_HPP
#define HTTPLIBSERVER_HPP


#include <httplib.h>
#include <google/protobuf/util/json_util.h>

#include "../IManagedServer.hpp"
#include "ServerGlobals.hpp"
#include "HttpLibServerLifeCycleManager.hpp"
#include "tools/HttpRestClient.hpp"


namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace httplib;

    struct ServerOptions {
        std::string host = "localhost";
        int port = 0;
    };

    class HttpLibServer final: public IManagedServer<HttpLibServer> {
        ServerOptions options_;
        Server server_;
        HttpLibServerLifeCycleManager life_cycle_manager_;
    public:
        explicit HttpLibServer(ServerOptions options = {})
            : options_(std::move(options)) {
            InitServer();
        }

        Server& GetHttpLibServer() {
            return server_;
        }

        void InitServer() {
            server_.Get("/health", [](const Request& req, Response& resp) {
                resp.set_content("ok", HTTP_CONTENT_TYPES.at(kPlainText));
                return true;
            });
            life_cycle_manager_.OnServerCreated(*this);
        }

        int Start() override {
            int port;
            if (options_.port > 0) {
                server_.bind_to_port(options_.host, options_.port);
                port = options_.port;
            } else {
                port = server_.bind_to_any_port(options_.host);
            }
            life_cycle_manager_.OnServerStart(*this, port);
            LOG_INFO("Server is up and running at port {}", port);
            return port;
        }

        void Shutdown() override {
            LOG_INFO("Server is shutting down");
            life_cycle_manager_.BeforeServerClose(*this);
            server_.stop();
            life_cycle_manager_.AfterServerClose(*this);
        }

        bool StartAndWait() override {
            Start();
            return server_.listen_after_bind();
        }

        void Use(const HttpLibControllerPtr &controller) override {
            life_cycle_manager_.AddServerLifeCylce(controller);
            controller->Mount(*this);
        }
    };
}




#endif //HTTPLIBSERVER_HPP
