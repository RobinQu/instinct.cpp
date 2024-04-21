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

    static void GracefullyShutdownRunningHttpServers();

    /**
     * un-managed pointers to `HttpLibServer`
     */
    static std::set<HttpLibServer*> RUNNING_HTTP_SERVERS;



    struct RestRouteSession {
        const Request request;
        Response& resp;
    };


    class HttpLibServer final: public IManagedServer<HttpLibServer> {
        ServerOptions options_;
        Server server_;
        HttpLibServerLifeCycleManager life_cycle_manager_;

    public:
        static void RegisterSignalHandlers() {
            static bool DONE = false;
            if (!DONE) { // register only once
                LOG_INFO("Regsiter signal handlers");
                DONE = true;
                std::signal(SIGINT, [](int signal) {
                    GracefullyShutdownRunningHttpServers();
                    std::exit(0);
                });
                std::signal(SIGTERM, [](int signal) {
                    GracefullyShutdownRunningHttpServers();
                    std::exit(0);
                });
            }
        }

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
            RUNNING_HTTP_SERVERS.insert(this);
            life_cycle_manager_.OnServerStart(*this, port);
            LOG_INFO("Server is up and running at port {}", port);
            return port;
        }

        void Shutdown() override {
            LOG_INFO("Server is shutting down");
            life_cycle_manager_.BeforeServerClose(*this);
            RUNNING_HTTP_SERVERS.erase(this);
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


        template<typename Req, typename Res, typename Fn, typename EntityConverter=ProtobufUtils>
        requires std::is_invocable_r_v<Res, Fn, const Req&, const RestRouteSession&>
        void PostRoute(const std::string& path, Fn&& fn) {
            GetHttpLibServer().Post(path, [&,fn](const Request& req, Response& resp) {
                const auto req_entity = EntityConverter::Deserialize(req.body);
                auto resp_entity = std::invoke(fn, req_entity, {req,resp});
                resp.body = EntityConverter::Serialize(resp_entity);
            });
        }

        template<typename Req, typename Res, typename Fn, typename EntityConverter=ProtobufUtils>
        requires std::is_invocable_r_v<Res, Fn, const Req&, const RestRouteSession&>
        void GetRoute(const std::string& path, Fn&& fn) {
            GetHttpLibServer().Get(path, [&,fn](const Request& req, Response& resp) {
                const auto req_entity = EntityConverter::Deserialize(req.body);
                auto resp_entity = std::invoke(fn, req_entity, {req,resp});
                resp.body = EntityConverter::Serialize(resp_entity);
            });
        }

    };

    static void GracefullyShutdownRunningHttpServers() {
        for (auto& server: RUNNING_HTTP_SERVERS) {
            server->Shutdown();
        }
    }
}




#endif //HTTPLIBSERVER_HPP
