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
                try {
                    std::rethrow_exception(ep);
                } catch (const ClientException& ex) {
                    LOG_ERROR("Exception caught in hanlder: {}, stacktrace \n", ex.message());
#ifdef NDEBUG
                    ex.trace().print(std::cerr);
#else
                    ex.trace().print_with_snippets(std::cerr);
#endif
                    HttpLibSession::Respond(res, ex.message(), 500);
                } catch(const InstinctException& ex) {
                    LOG_ERROR("Exception caught in hanlder: {}, stacktrace \n", ex.message());
#ifdef NDEBUG
                    ex.trace().print(std::cerr);
#else
                    ex.trace().print_with_snippets(std::cerr);
#endif
                    HttpLibSession::Respond(res, ex.message(), 500);
                } catch (const cpptrace::exception& ex) {
                    LOG_ERROR("Exception caught in hanlder: {}, stacktrace \n", ex.message());
#ifdef NDEBUG
                    ex.trace().print(std::cerr);
#else
                    ex.trace().print_with_snippets(std::cerr);
#endif
                    HttpLibSession::Respond(res, ex.message(), 500);
                } catch (const std::exception& ex) {
                    LOG_ERROR("Uncaught std::excetion found. ex.what={}", ex.what());
                    HttpLibSession::Respond(res, ex.what(), 500);
                } catch (...) {
                    HttpLibSession::Respond(res, "Unkown exception occured.", 500);
                }
            });
        }
    };
}


#endif //DEFAULTERRORCONTROLLER_HPP
