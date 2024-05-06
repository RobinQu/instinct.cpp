//
// Created by RobinQu on 2024/5/6.
//

#ifndef DEFAULTERRORCONTROLLER_HPP
#define DEFAULTERRORCONTROLLER_HPP

#include "HttpLibServer.hpp"
#include "../IMountable.hpp"

namespace
INSTINCT_SERVER_NS {
    class DefaultErrorController final : public IMountable<HttpLibServer> {
    public:
        void Mount(HttpLibServer &server) override {
            LOG_INFO("Mount DefaultErrorController");
            server.GetHttpLibServer().set_exception_handler([](const auto &req, auto &res, const std::exception_ptr &ep) {
                nlohmann::json error_response;
                try {
                    std::rethrow_exception(ep);
                } catch (const ClientException& ex) {
                    error_response["message"] = ex.what();
                    res.status = StatusCode::BadRequest_400;
                } catch (const std::exception& ex) {
                    LOG_ERROR("Uncaought excetion found. ex.what={}", ex.what());
                    res.status = StatusCode::InternalServerError_500;
                    error_response["message"] = ex.what();
                } catch (...) {
                    res.status = StatusCode::InternalServerError_500;
                    error_response["message"] = "Unkown exception occured.";
                }
                res.set_content(error_response.dump(), HTTP_CONTENT_TYPES.at(kJSON));

            });
        }
    };
}


#endif //DEFAULTERRORCONTROLLER_HPP
