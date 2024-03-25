//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_ICONTEXT_HPP
#define INSTINCT_ICONTEXT_HPP

#include <memory>
#include "CoreGlobals.hpp"
#include "tools/Assertions.hpp"



namespace INSTINCT_CORE_NS {
    template<typename ContextPolicy>
    class IContext;

    template<typename ContextPolicy>
    using ContextPtr = std::shared_ptr<IContext<ContextPolicy>>;


    template<typename ContextPolicy>
    using MappingContext = std::unordered_map<std::string, ContextPtr<ContextPolicy>>;

    template<typename ContextPolicy>
    class IContext final {
        ContextPolicy policy_;
    public:

        explicit IContext(ContextPolicy policy) : policy_(policy) {}

        const ContextPolicy::ValueType& GetValue() const {
            return policy_.GetValue();
        }

        template<typename T>
        T RequirePrimitive() {
            return policy_.template RequirePrimitive<T>();
        }

        template<typename T>
        void ProducePrimitive(T&& value) {
            policy_.template PutValue<T>(std::forward<T>(value));
        }

        template<typename T>
        [[nodiscard]] T RequireMessage() const {
            return policy_.template RequireMessage<T>();
        }

        template<typename T>
        void ProduceMessage(T&& message) {
            policy_.PutMessage(std::forward<T>(message));
        }

        template<typename T>
        void ProduceMessage(const T& message) {
            policy_.PutMessage(message);
        }

        void ProduceMappingData(const MappingContext<ContextPolicy>& mapping_data) {
            policy_.PutMappingObject(mapping_data);
        }

        [[nodiscard]] MappingContext<ContextPolicy> RequireMappingData() const {
            return policy_.GetMappingObject();
        }

        [[nodiscard]] bool IsPrimitive() const {
            return policy_.IsPrimitive();
        }

        [[nodiscard]] bool IsMessage() const {
            return policy_.IsMessage();
        }

        [[nodiscard]] bool IsMappingObject() const {
            return policy_.IsMappingObject();
        }

    };

}



#endif //INSTINCT_ICONTEXT_HPP
