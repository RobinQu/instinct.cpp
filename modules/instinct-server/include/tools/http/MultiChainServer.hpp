//
// Created by RobinQu on 3/19/24.
//

#ifndef INSTINCT_MULTICHAINSERVER_HPP
#define INSTINCT_MULTICHAINSERVER_HPP

#include <string>
#include <utility>
#include <httplib.h>
#include <google/protobuf/util/json_util.h>

#include "HttpLibServer.hpp"
#include "ServerGlobals.hpp"
#include "functional/StepFunctions.hpp"
#include "tools/HttpRestClient.hpp"

namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_CORE_NS;
    using namespace httplib;

    class MultiChainServer final: public HttpLibServer {
        std::unordered_set<std::string> chain_names_;
    public:
        explicit MultiChainServer(ServerOptions options = {})
            : HttpLibServer(std::move(options)) {
        }

        int Start() override {
            int port = HttpLibServer::Start();
            LOG_INFO("ChainServer is up and running at {} with {} registered chain(s)", port, chain_names_.size());
            if (chain_names_.empty()) {
                LOG_WARN("ChainServer has no chains registered yet!");
            }
            return port;
        }

        bool AddNamedChain(const std::string &chain_name, const GenericChainPtr &chain) {
            if (chain_names_.contains(chain_name)) {
                return false;
            }
            chain_names_.insert(chain_name);
            LOG_INFO("AddNamedChain: name={}", chain_name);
            GetHttpLibServer().Post(fmt::format("/chains/{}/invoke", chain_name), [&, chain](const Request &req, Response &resp) {
                LOG_DEBUG("POST /chains/{}/invoke -->", chain_name);
                auto context = CreateJSONContextWithString(req.body);
                auto result = chain->Invoke(context);

                resp.set_content(
                        DumpJSONContext(result),
                        HTTP_CONTENT_TYPES.at(kJSON)
                );
            });

            GetHttpLibServer().Post(fmt::format("/chains/{}/batch", chain_name), [&, chain](const Request &req, Response &resp) {
                LOG_DEBUG("POST /chains/{}/batch -->", chain_name);

                auto batch_context = CreateBatchJSONContextWithString(req.body);
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

            GetHttpLibServer().Post(fmt::format("/chains/{}/stream", chain_name), [&,chain](const Request& req, Response& resp) {
                LOG_DEBUG("POST /chains/{}/stream -->", chain_name);
                auto context = CreateJSONContextWithString(req.body);
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

            return true;
        }


    };

}


#endif //INSTINCT_MULTICHAINSERVER_HPP
