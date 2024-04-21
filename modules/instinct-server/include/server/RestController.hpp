//
// Created by RobinQu on 2024/4/21.
//

#ifndef RESTCONTROLLER_HPP
#define RESTCONTROLLER_HPP

#include <functional>
#include <httplib.h>

#include "HttpController.hpp"
#include "IManagedServer.hpp"
#include "ServerGlobals.hpp"

namespace INSTINCT_SERVER_NS {
    template<typename ServerImpl>
    requires std::derived_from<ServerImpl, IManagedServer<ServerImpl>>
    class RestRoutes {

        template<typename Req, typename Res, typename Fn>
        requires std::is_invocable_r_v<Res, Fn, Req>
        static AddPost(ServerImpl &server, std::string& path, Fn&& fn) {
            server.GetHttpLibServer().Post(path, [](const httplib::Request& req, httplib::Response& resp) {

            });
        }
    };



    template<typename ServerImpl>
    class RestController final: HttpController<ServerImpl> {

    public:
        template<typename Req, typename Res, typename Fn>
        requires std::is_invocable_r_v<Res, Fn, Req>
        void PostRoute(const std::string& path, Fn fn) {

        }

    };
}

#endif //RESTCONTROLLER_HPP
