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

        const ContextPolicy::PayloadType& GetPayload() const {
            return payload_;
        }

        ContextPolicy::PayloadType& GetPayload() {
            return payload_;
        }

        template<typename T>
        T RequirePrimitive() {

        }

        template<typename T>
        T RequireMessage() {

        }

        template<typename T>
        void ProduceMessage(const T& message) {

        }

        template<typename T>
        void ProducePrimitive(const T& value) {

        }

        template<typename T>
        T RequirePrimitive(const std::string& name) const {
            return ContextPolicy::ManagerType::template RequirePrimitive<T>(payload_, name);
        }

        template<typename T>
        T RequireMessage(const std::string& name) const {
            return ContextPolicy::ManagerType::template RequireMessage<T>(payload_, name);
        }

//        template<typename T>
//        void PutMessage(const std::string& name, T&& message) {
//
//        }

        template<typename T>
        void PutMessage(const std::string& name, const T& message) {
            return ContextPolicy::ManagerType::template PutMessage<T>(payload_, name, message);
        }

        [[nodiscard]] bool Contains(const std::string& name) const {
            return ContextPolicy::ManagerType::Contains(payload_, name);
        }

        template<typename T>
        T RequirePrimitiveAt(const std::string& data_path) const {
            return ContextPolicy::ManagerType::template RequirePrimitiveAt<T>(payload_, data_path);
        }

        template<typename T>
        void PutPrimitive(const std::string& name, T&& value) {
            ContextPolicy::ManagerType::template PutPrimitive<T>(payload_, name, std::forward<T>(value));
        }

        template<typename T>
        void PutPrimitiveAt(const std::string& data_path, T&& value) {
            ContextPolicy::ManagerType::template PutPrimitiveAt<T>(payload_, data_path, std::forward<T>(value));
        }

        void PutContext(const std::string& name, const IContext<ContextPolicy>& child_context) {
            ContextPolicy::ManagerType::PutObject(payload_, name, child_context.payload_);
        }

        void MergeContext(const IContext<ContextPolicy>& child_context) {
            ContextPolicy::ManagerType::MergeObject(payload_, child_context.payload_);
        }

    };

}



#endif //INSTINCT_ICONTEXT_HPP
