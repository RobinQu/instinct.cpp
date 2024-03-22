//
// Created by RobinQu on 3/19/24.
//

#ifndef INSTINCT_PLAINCHAINSERVER_HPP
#define INSTINCT_PLAINCHAINSERVER_HPP

#include <string>
#include <utility>
#include <httplib.h>
#include <google/protobuf/util/json_util.h>

#include "LLMGlobals.hpp"
#include "ServerGlobals.hpp"
#include "chain/MessageChain.hpp"
#include "tools/HttpRestClient.hpp"

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_CORE_NS;
    using namespace httplib;

    struct ServerOptions {
        std::string host = "localhost";
        int port = 0;
    };

    template<typename HttpEntityConverter = ProtobufHttpEntityConverter>
    class PlainChainServer {
        std::unordered_set<std::string> chain_names_;
        HttpEntityConverter converter_;
        ServerOptions options_;
        Server server_;
    public:

        explicit PlainChainServer(ServerOptions options = {}) : options_(std::move(options)) {}

        template<class T>
        bool AddNamedChain(const std::string &chain_name, const MessageChainPtr<T> &chain) {
            if (chain_names_.contains(chain_name)) {
                return false;
            }
            chain_names_.insert(chain_name);
            LOG_INFO("AddNamedChain: name={}", chain_name);
            server_.Post(fmt::format("/chains/{}/invoke", chain_name), [&, chain](const Request &req, Response &resp) {
                LOG_DEBUG("POST /chains/{}/invoke -->", chain_name);
                auto context = CreateJSONContextWithString(req.body);
                auto result = chain->Invoke(context);

                resp.set_content(
                        converter_.Serialize(result),
                        HTTP_CONTENT_TYPES.at(kJSON)
                );
            });

            server_.Post(fmt::format("/chains/{}/batch", chain_name), [&, chain](const Request &req, Response &resp) {
                LOG_DEBUG("POST /chains/{}/batch -->", chain_name);
                auto batch_context = CreateBatchJSONContextWithString(req.body);
                std::vector<std::string> parts;
                chain->Batch(batch_context)
                    | rpp::operators::as_blocking()
                    | rpp::operators::map([&](const auto& item) {
                        return converter_.Serialize(item);
                    })
                    | rpp::operators::subscribe([&](const std::string& line) {
                        parts.push_back(line);
                    });
                resp.set_content(
                        "[" + StringUtils::JoinWith(parts, ", ") + "]",
                        HTTP_CONTENT_TYPES.at(kJSON)
                );
            });

            server_.Post(fmt::format("/chains/{}/stream", chain_name), [&,chain](const Request& req, Response& resp) {
                LOG_DEBUG("POST /chains/{}/stream -->", chain_name);
                auto context = CreateJSONContextWithString(req.body);
                AsyncIterator<T> itr = chain->Stream(context);
                resp.set_chunked_content_provider(HTTP_CONTENT_TYPES.at(kEventStream), [&, chain] (size_t offset, DataSink &sink) {
                    itr.subscribe(
                            [&](const T &item) {// on_next
                                std::string buf = converter_.Serialize(item);
                                sink.write(buf.data(), buf.size());
                            },
                            {},//on_error
                            [&]() { // on_complete
                                sink.done();
                            }
                    );
                    return true;
                });
            });

            return true;
        }

        void InitServer() {
            server_.Get("/health", [](const Request& req, Response& resp) {
                resp.set_content("ok", HTTP_CONTENT_TYPES.at(kPlainText));
                return true;
            });
        }

        int Start() {
            int port;
            if (options_.port > 0) {
                server_.bind_to_port(options_.host, options_.port);
                port = options_.port;
            }
            port = server_.bind_to_any_port(options_.host);
            LOG_INFO("ChainServer is up and running at {} with {} registered chain(s)", port, chain_names_.size());
            if (chain_names_.empty()) {
                LOG_WARN("ChainServer has no chains registered yet!");
            }
            return port;
        }

        void Shutdown() {
            LOG_INFO("ChainServer is shutting down");
            server_.stop();
        }

        bool StartAndWait() {
            Start();
            return server_.listen_after_bind();
        }

    };

}


#endif //INSTINCT_PLAINCHAINSERVER_HPP
