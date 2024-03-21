//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_ICONTEXT_HPP
#define INSTINCT_ICONTEXT_HPP

#include <memory>
#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS {
    template<typename ContextPolicy>
    class IContext;

    template<typename ContextPolicy>
    using ContextPtr = std::shared_ptr<IContext<ContextPolicy>>;

    template<typename ContextPolicy>
    class IContext final {

//        ContextPolicy::ManagerType manager_;
        ContextPolicy::PayloadType payload_;
    public:

        IContext(IContext&& context)  noexcept {
            this->payload_ = std::move(context.payload_);
        }

        IContext(const IContext& context) {
            this->payload_ = context.payload_;
        }

        explicit IContext(ContextPolicy::PayloadType payload) : payload_(std::move(payload)) {

        }

        template<typename T>
        T RequirePrimitive(const std::string& name) const {
            return ContextPolicy::ManagerType::RequirePrimitive(payload_, name);
        }

        template<typename T>
        T RequirePrimitiveAt(const std::string& data_path) const {
            return ContextPolicy::ManagerType::RequirePrimitiveAt(payload_, data_path);
        }

        template<typename T>
        IContext* PutPrimitive(const std::string& name, T&& value) {
            ContextPolicy::ManagerType::PutPrimitive(payload_, name, std::forward<T>(value));
            return this;
        }

        template<typename T>
        IContext* PutPrimitiveAt(const std::string& data_path, T&& value) {
            ContextPolicy::ManagerType::PutPrimitiveAt(payload_, data_path, std::forward<T>(value));
            return this;
        }

        IContext* PutContext(const std::string& name, const IContext<ContextPolicy>& child_context) {
            ContextPolicy::ManagerType::PutObject(payload_, name, child_context.payload_);
            return this;
        }

        IContext* MergeContext(const IContext<ContextPolicy>& child_context) {
            ContextPolicy::ManagerType::MergeObject(payload_, child_context.payload_);
            return this;
        }

    };

}



#endif //INSTINCT_ICONTEXT_HPP
