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
#include "HttpLibSession.hpp"

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
    static std::vector<std::weak_ptr<HttpLibServer>> RUNNING_HTTP_SERVERS;


    using HttpLibController = HttpController<HttpLibServer>;
    using HttpLibControllerPtr = std::shared_ptr<HttpLibController>;

    class HttpLibServer final: public IManagedServer<HttpLibServer>, public std::enable_shared_from_this<HttpLibServer> {
        ServerOptions options_;
        Server server_;
        HttpLibServerLifeCycleManager life_cycle_manager_;


        class log_guard {
            std::string method_;
            std::string path_;
        public:
            log_guard(std::string method, std::string path)
                : method_(std::move(method)),
                  path_(std::move(path)) {
                LOG_DEBUG("--> {} {}", method_, path_);
            }

            ~log_guard() {
                LOG_DEBUG("<-- {} {}", method_, path_);
            }
        };


    public:
        static void RegisterSignalHandlers() {
            static bool DONE = false;
            if (!DONE) { // register only once
                LOG_INFO("Register signal handlers");
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
            RUNNING_HTTP_SERVERS.push_back(this->shared_from_this());
            life_cycle_manager_.OnServerStart(*this, port);
            LOG_INFO("Server is up and running at port {}", port);
            return port;
        }

        void Shutdown() override {
            LOG_INFO("Server is shutting down");
            life_cycle_manager_.BeforeServerClose(*this);
            RUNNING_HTTP_SERVERS.push_back(this->shared_from_this());
            server_.stop();
            life_cycle_manager_.AfterServerClose(*this);
        }

        bool StartAndWait() override {
            Start();
            return server_.listen_after_bind();
        }

        void Use(const MountablePtr<HttpLibServer> &mountable) override {
            if (const auto controller = std::dynamic_pointer_cast<HttpLibController>(mountable)) {
                life_cycle_manager_.AddServerLifeCylce(controller);
                controller->Mount(*this);
            } else {
                mountable->Mount(*this);
            }
        }


        template<typename Req, typename Res, typename Fn, typename EntityConverter=ProtobufUtils>
        requires std::is_invocable_r_v<void, Fn, Req&, HttpLibSession&>
        void PostRoute(const std::string& path, Fn&& fn) {
            LOG_INFO("Route added:  POST {}", path);
            GetHttpLibServer().Post(path, [&,fn,path](const Request& req, Response& resp) {
                log_guard log {"POST", path};
                assert_not_blank(req.body, "request body cannot be empty");
                auto req_entity = EntityConverter::template Deserialize<Req>(req.body);
                const HttpLibSession session {req, resp};
                CPPTRACE_WRAP_BLOCK(
                    std::invoke(fn, req_entity, session);
                );
            });
        }

        template<typename Req, typename Res, typename Fn, typename EntityConverter=ProtobufUtils>
        requires std::is_invocable_r_v<void, Fn, Req&, HttpLibSession&>
        void GetRoute(const std::string& path, Fn&& fn) {
            LOG_INFO("Route added:  GET {}", path);
            GetHttpLibServer().Get(path, [&,fn,path](const Request& req, Response& resp) {
                log_guard log {"GET", path};
                const HttpLibSession session {req, resp};
                Req req_entity;
                CPPTRACE_WRAP_BLOCK(
                    std::invoke(fn, req_entity, session);
                );
            });
        }

        template<typename Req, typename Res, typename Fn, typename EntityConverter=ProtobufUtils>
        requires std::is_invocable_r_v<void, Fn, Req&, HttpLibSession&>
        void DeleteRoute(const std::string& path, Fn&& fn) {
            LOG_INFO("Route added:  DELETE {}", path);
            GetHttpLibServer().Delete(path, [&,fn,path](const Request& req, Response& resp) {
                log_guard log {"DELETE", path};
                const HttpLibSession session {req, resp};
                Req req_entity;
                CPPTRACE_WRAP_BLOCK(
                    std::invoke(fn, req_entity, session);
                );
            });
        }

    };

    using HttpLibServerPtr = std::shared_ptr<HttpLibServer>;

    static void GracefullyShutdownRunningHttpServers() {
        for (auto& server: RUNNING_HTTP_SERVERS) {
            if (const auto ptr = server.lock()) {
                ptr->Shutdown();
            }
        }
    }
}




#endif //HTTPLIBSERVER_HPP
