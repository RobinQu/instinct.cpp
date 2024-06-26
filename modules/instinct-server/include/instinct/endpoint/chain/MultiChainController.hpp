//
// Created by RobinQu on 3/19/24.
//

#ifndef INSTINCT_MULTICHAINSERVER_HPP
#define INSTINCT_MULTICHAINSERVER_HPP

#include <string>
#include <httplib.h>
#include <google/protobuf/util/json_util.h>

#include <instinct/ServerGlobals.hpp>
#include <instinct/functional/StepFunctions.hpp>
#include <instinct/server/HttpController.hpp>
#include <instinct/server/httplib/HttpLibServer.hpp>
#include <instinct/tools/HttpRestClient.hpp>

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_CORE_NS;
    using namespace httplib;

    class MultiChainController final: public HttpLibController {
        std::unordered_map<std::string, GenericChainPtr> chains_;
    public:

        void OnServerStart(HttpLibServer &server, int port) override {
            LOG_INFO("ChainServer is up and running at {} with {} registered chain(s)", port, chains_.size());
            if (chains_.empty()) {
                LOG_WARN("ChainServer has no chains registered yet!");
            }
        }


        void Mount(HttpLibServer &server) override {
            for(const auto& [chain_name, chain]: chains_) {
                LOG_INFO("AddNamedChain: name={}", chain_name);
                server.GetHttpLibServer().Post(fmt::format("/chains/{}/invoke", chain_name), [&, chain](const Request &req, Response &resp) {
                LOG_DEBUG("POST /chains/{}/invoke -->", chain_name);
                const auto context = CreateJSONContextWithString(req.body);
                const auto result = chain->Invoke(context);
                resp.set_content(
                        DumpJSONContext(result),
                        HTTP_CONTENT_TYPES.at(kJSON)
                );
            });

            server.GetHttpLibServer().Post(fmt::format("/chains/{}/batch", chain_name), [&, chain](const Request &req, Response &resp) {
                LOG_DEBUG("POST /chains/{}/batch -->", chain_name);
                const auto batch_context = CreateBatchJSONContextWithString(req.body);
                std::vector<std::string> parts;
                parts.reserve(batch_context.size());
                chain->Batch(batch_context)
                    | rpp::operators::as_blocking()
                    | rpp::operators::map([&](const JSONContextPtr& item) {
                        return DumpJSONContext(item);
                    })
                    | rpp::operators::subscribe([&](const std::string& line) {
                        parts.push_back(line);
                    });
                resp.set_content(
                        "[" + StringUtils::JoinWith(parts, ", ") + "]",
                        HTTP_CONTENT_TYPES.at(kJSON)
                );
            });

            server.GetHttpLibServer().Post(fmt::format("/chains/{}/stream", chain_name), [&,chain](const Request& req, Response& resp) {
                LOG_DEBUG("POST /chains/{}/stream -->", chain_name);
                const auto context = CreateJSONContextWithString(req.body);
                AsyncIterator<JSONContextPtr> itr = chain->Stream(context);
                resp.set_chunked_content_provider(HTTP_CONTENT_TYPES.at(kEventStream), [&, chain] (size_t offset, DataSink &sink) {
                    itr.subscribe(
                            [&](const JSONContextPtr &item) {// on_next
                                std::string buf = DumpJSONContext(item);
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
            }
        }

        bool AddNamedChain(const std::string &chain_name, const GenericChainPtr &chain) {
            if (chains_.contains(chain_name)) {
                return false;
            }
            chains_[chain_name] = chain;;
            return true;
        }
    };

    static std::shared_ptr<MultiChainController> CreateMultiChainController() {
        return std::make_shared<MultiChainController>();
    }

}


#endif //INSTINCT_MULTICHAINSERVER_HPP
