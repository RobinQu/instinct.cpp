//
// Created by RobinQu on 2024/1/10.
//

#ifndef DOCUMENT_H
#define DOCUMENT_H
#include <memory>
#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"

namespace INSTINCT_CORE_NS {
    using DocumentId = u_int64_t;

    using FieldValue = std::variant<std::monostate, int64_t, u_int64_t, float, double, std::string, bool>;
    enum FieldType {
        BOOL,
        INT,
        BIGINT,
        FLOAT,
        DOUBLE,
        VARCHAR,
        FLOAT_VECTOR
    };

    struct FieldSchema {
        FieldType field_type;
        std::string descrition;
        bool required;
        FieldValue default_value;
    };
    using DocumentSchema = std::unordered_map<std::string, FieldSchema>;

    struct Field {
        std::string name;
        FieldValue value;

        [[nodiscard]] int64_t AsBigInt() const {
            return std::get<int64_t>(value);
        }
    };

    struct Document {
        DocumentId id;
        std::string text;
        std::map<std::string, FieldValue> metadata;
    };

    //
    // class DocumentMetadata {
    //     std::unordered_map<std::string, MetadataValueVariant> metadata_;
    // public:
    //     void PutValue(const std::string& k, MetadataValueVariant value);
    //     long GetIntegerValue(const std::string& k);
    //     bool GetBooleanValue(const std::string& k);
    //     double GetDoubleValue(const std::string& k);
    //     std::string GetStringValue(const std::string& k);
    //
    //     template<typename T>
    //     static DocumentMetadata FromStruct(T &&t) {
    //         // requries conversion constructor or operator.
    //         return static_cast<DocumentMetadata>(std::forward<T>(t));
    //     }
    // };


    using DocumentPtr = std::shared_ptr<Document>;
}

#endif //DOCUMENT_H
