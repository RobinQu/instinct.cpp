//
// Created by RobinQu on 2024/6/27.
//

#ifndef SERVER_ALL_HPP
#define SERVER_ALL_HPP
#include <instinct/endpoint/chain/multi_chain_controller.hpp>
#include <instinct/endpoint/chat_completion/chat_completion_controller.hpp>
#include <instinct/endpoint/chat_completion/chat_completion_request_input_parser.hpp>
#include <instinct/endpoint/chat_completion/chat_completion_response_output_parser.hpp>
#include <instinct/server/HttpController.hpp>
#include <instinct/server/http_server_exception.hpp>
#include <instinct/server/httplib/default_error_controller.hpp>
#include <instinct/server/httplib/http_lib_server.hpp>
#include <instinct/server/httplib/http_lib_server_lifecycle_manager.hpp>
#include <instinct/server/httplib/http_lib_session.hpp>
#include <instinct/server/managed_server.hpp>
#include <instinct/server/mountable.hpp>
#include <instinct/server/server_lifecycle_handler.hpp>
#include <instinct/server_all.hpp>
#include <instinct/server_global.hpp>

#endif //SERVER_ALL_HPP
