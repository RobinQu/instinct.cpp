//
// Created by RobinQu on 2024/5/6.
//

#ifndef DEFAULTERRORCONTROLLER_HPP
#define DEFAULTERRORCONTROLLER_HPP

#include <iostream>
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
                    LOG_ERROR("Exception caught with stacktrace:");
#ifdef NDEBUG
                    ex.trace().print(std::cerr);
#else
                    ex.trace().print_with_snippets(std::cerr);
#endif
                    error_response["message"] = ex.message();
                    res.status = StatusCode::BadRequest_400;
                } catch(const InstinctException& ex) {
                    LOG_ERROR("Exception caught with stacktrace:");
#ifdef NDEBUG
                    ex.trace().print(std::cerr);
#else
                    ex.trace().print_with_snippets(std::cerr);
#endif
                    error_response["message"] = ex.message();
                    res.status = StatusCode::InternalServerError_500;
                } catch (const cpptrace::exception& ex) {
                    LOG_ERROR("Exception caught with stacktrace:");
#ifdef NDEBUG
                    ex.trace().print(std::cerr);
#else
                    ex.trace().print_with_snippets(std::cerr);
#endif
                    error_response["message"] = ex.message();
                    res.status = StatusCode::InternalServerError_500;
                } catch (const std::exception& ex) {
                    LOG_ERROR("Uncaought std::excetion found. ex.what={}", ex.what());
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
