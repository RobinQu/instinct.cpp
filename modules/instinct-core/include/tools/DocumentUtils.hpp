//
// Created by RobinQu on 2024/3/11.
//

#ifndef DOCUMENTUTILS_HPP
#define DOCUMENTUTILS_HPP


#include <google/protobuf/util/message_differencer.h>
#include "StringUtils.hpp"


namespace std {
    /**
     * hash only using doc.id()
     */
    template <>
    struct hash<INSTINCT_CORE_NS::Document> {
        size_t operator()(const INSTINCT_CORE_NS::Document& doc) const noexcept {
            return std::hash<std::string_view> {} (doc.id());
        }
    };

    /**
     * compare two documents using `Equivalent` sematics.
     */
    template<>
    struct equal_to<INSTINCT_CORE_NS::Document> {
        bool operator()(const INSTINCT_CORE_NS::Document& lhs, const INSTINCT_CORE_NS::Document& rhs) const {
            return google::protobuf::util::MessageDifferencer::Equivalent(lhs, rhs);
        }
    };

}

namespace INSTINCT_CORE_NS {
    const static std::string ROOT_DOC_ID = "49915DBE-434A-4E3E-9E0A-791D2F69D386";

    struct DocumentMetadataMutator {
        Document* document_;

        auto SetString(const std::string& name, const std::string& value) {
            auto* field = document_->add_metadata();
            field->set_name(name);
            field->set_string_value(value);
            return this;
        }

        auto SetInt32(const std::string& name, const int& value) {
            auto* field = document_->add_metadata();
            field->set_name(name);
            field->set_int_value(value);
            return this;
        }
    };


    enum ConstraintViolationCategory {
        kUnknownViolation,
        kIllegalFieldValue,
        kIllegalMetadataFormat,
    };

    struct ConstraintViolation {
        ConstraintViolationCategory category = kIllegalFieldValue;
        std::string field_name;
        std::string message;
    };

    /**
     * Empty value for MetadataSchema
     */
    static auto EMPTY_METADATA_SCHEMA = std::make_shared<MetadataSchema>();


    class DocumentUtils {
    public:
        static std::string CombineDocuments(const AsyncIterator<Document>& doc_itr) {
            std::string buf;
            doc_itr
                | rpp::operators::as_blocking()
                | rpp::operators::subscribe([&](const Document& doc) {
                    buf += doc.text();
                    buf += "\n";
                })
            ;
            return buf;
        }

        static bool HasMetadataField(const Document& document, const std::string& name) {
            return std::ranges::any_of(document.metadata(), [&](const auto& m) {
                return m.name() == name;
            });
        }

        static  RepeatedPtrField<PrimitiveVariable>::const_iterator GetMetadataFieldValue(const Document& document, const std::string& name) {
            for(auto itr=document.metadata().begin(); itr!=document.metadata().end();++itr) {
                if (itr->name() == name) {
                    return itr;
                }
            }
            return document.metadata().end();
        }

        static void AddMissingPresetMetadataFields(Document& document) {
            if(!HasMetadataField(document, METADATA_SCHEMA_PARENT_DOC_ID_KEY)) {
                auto* metadata_field = document.add_metadata();
                metadata_field->set_name(METADATA_SCHEMA_PARENT_DOC_ID_KEY);
                metadata_field->set_string_value("");
            }
            if(!HasMetadataField(document, METADATA_SCHEMA_FILE_SOURCE_KEY)) {
                auto* metadata_field = document.add_metadata();
                metadata_field->set_name(METADATA_SCHEMA_FILE_SOURCE_KEY);
                metadata_field->set_string_value("");
            }
            if(!HasMetadataField(document, METADATA_SCHEMA_PAGE_NO_KEY)) {
                auto* metadata_field = document.add_metadata();
                metadata_field->set_name(METADATA_SCHEMA_PAGE_NO_KEY);
                metadata_field->set_int_value(0);
            }
        }

        static void AddPresetMetadataFields(
            Document& document,
            const std::string& parent_doc_id,
            const int page_no = 1,
            const std::string& source = "",
            const std::string& parent_doc_id_key = METADATA_SCHEMA_PARENT_DOC_ID_KEY,
            const std::string& page_no_key = METADATA_SCHEMA_PAGE_NO_KEY,
            const std::string& source_key = METADATA_SCHEMA_FILE_SOURCE_KEY
            ) {
            auto* parent_id_field = document.add_metadata();
            parent_id_field->set_name(parent_doc_id_key);
            parent_id_field->set_string_value(parent_doc_id);
            auto *page_no_field = document.add_metadata();
            page_no_field->set_name(page_no_key);
            page_no_field->set_int_value(page_no);
            auto *source_field = document.add_metadata();
            source_field->set_name(source_key);
            source_field->set_string_value(source);
        }

        static std::vector<ConstraintViolation> ValidateDocument(const Document& doc, const MetadataSchemaPtr& metadata_schema, bool bypass_unknown_fields = true) {
            std::vector<ConstraintViolation> violations;

            if(StringUtils::IsBlankString(doc.text())) {
                violations.push_back({.field_name = "text", .message = "text cannot be blank" });
                return violations;
            }

            // if empty metadata is given
            if (!metadata_schema || metadata_schema == EMPTY_METADATA_SCHEMA || metadata_schema->fields_size() == 0) {
                return violations;
            }

            auto name_view = metadata_schema->fields() | std::views::transform(
                    [](const MetadataFieldSchema& field)-> std::string {
                        return field.name();
                    });
            std::unordered_set<std::string> known_field_names{name_view.begin(), name_view.end()};

            auto* schema = Document::GetDescriptor();
            auto* reflection = Document::GetReflection();
            const auto* metadata_field = schema->FindFieldByName("metadata");
            int metadata_size = reflection->FieldSize(doc, metadata_field);
            auto field_schema_map = DocumentUtils::ConvertToMetadataSchemaMap(metadata_schema);
            for (int i = 0; i < metadata_size; i++) {
                const auto &metadata_field_message = reflection->GetRepeatedMessage(
                        doc,
                        metadata_field,
                        i);
                auto metadata_field_descriptor = metadata_field_message.GetDescriptor();

                auto name_field_descriptor = metadata_field_descriptor->FindFieldByName("name");
                auto* field_value_reflection = metadata_field_message.GetReflection();
                auto name = field_value_reflection->GetString(metadata_field_message, name_field_descriptor);

                // TODO keep following logs for debugging purpose

//                for (int j = 0; j < metadata_field_descriptor->field_count(); ++j) {
//                    auto * metadata_field_desc = metadata_field_descriptor->field(j);
//                            LOG_DEBUG("name = {}", metadata_field_desc->name());
//                            LOG_DEBUG("type = {}", metadata_field_desc->type_name());
//
//                    if(auto* oneof_desc = metadata_field_desc->containing_oneof()) {
//                                LOG_DEBUG("oneof_desc->index() = {}", oneof_desc->index());
//
//                        auto index_in_oneof = metadata_field_desc->index_in_oneof();
//                                LOG_DEBUG("index_in_oneof = {}", index_in_oneof);
//                    }
//                }

                if (!bypass_unknown_fields) {
                    if (!field_schema_map.contains(name)) {
                        violations.push_back({.field_name = name,.message="Metadata cannot contain field not defined by schema."});
                        return violations;
                    }
                }

                // remove in names set to mark it as found
                known_field_names.erase(name);
                auto* value_descriptor = metadata_field_descriptor->FindOneofByName("value");

                if (!value_descriptor) {
                    violations.push_back({.category=kIllegalFieldValue, .field_name=name, .message="Field value is not set."});
                    return violations;
                }

                if (metadata_schema->fields_size() > 0 && !field_value_reflection) {
                    violations.push_back({.category = kIllegalMetadataFormat, .message = "Metadata is not set"});
                    return violations;
                }



                auto* field_schema = field_schema_map.at(name);
                auto* value_field_descriptor = field_value_reflection->GetOneofFieldDescriptor(metadata_field_message, value_descriptor);

                if(!value_field_descriptor) {
                    violations.push_back({.category=kIllegalFieldValue, .field_name=name, .message="Field value is not set."});
                    return violations;
                }

                switch (field_schema->type()) {
                    case VARCHAR: {
                        if (value_field_descriptor->type() != google::protobuf::FieldDescriptor::TYPE_STRING) {
                            violations.push_back({.category=kIllegalFieldValue, .field_name=name, .message=fmt::format("Field {} should have value of type string, but value of {} is given.", name, value_field_descriptor->type_name())});
                            return violations;
                        }
                        break;
                    }
                    case INT32: {
                        if (value_field_descriptor->type() != google::protobuf::FieldDescriptor::TYPE_INT32) {
                            violations.push_back({.category=kIllegalFieldValue, .field_name=name, .message=fmt::format("Field {} should have value of type int32, but value of {} is given.", name, value_field_descriptor->type_name())});
                            return violations;
                        }
                        break;
                    }
                    case INT64: {
                        if (value_field_descriptor->type() != google::protobuf::FieldDescriptor::TYPE_INT64) {
                            violations.push_back({.category=kIllegalFieldValue, .field_name=name, .message=fmt::format("Field {} should have value of type int64, but value of {} is given.", name, value_field_descriptor->type_name())});
                            return violations;
                        }
                        break;
                    }
                    case FLOAT: {
                        if (value_field_descriptor->type() != google::protobuf::FieldDescriptor::TYPE_FLOAT) {
                            violations.push_back({.category=kIllegalFieldValue, .field_name=name, .message=fmt::format("Field {} should have value of type float, but value of {} is given.", name, value_field_descriptor->type_name())});
                            return violations;
                        }
                        break;
                    }
                    case DOUBLE: {
                        if (value_field_descriptor->type() != google::protobuf::FieldDescriptor::TYPE_DOUBLE) {
                            violations.push_back({.category=kIllegalFieldValue, .field_name=name, .message=fmt::format("Field {} should have value of type double, but value of {} is given.", name, value_field_descriptor->type_name())});
                            return violations;
                        }
                        break;
                    }
                    case BOOL: {
                        if (value_field_descriptor->type() != google::protobuf::FieldDescriptor::TYPE_BOOL) {
                            violations.push_back({.category=kIllegalFieldValue, .field_name=name, .message=fmt::format("Field {} should have value of type bool, but value of {} is given.", name, value_field_descriptor->type_name())});
                            return violations;
                        }
                        break;
                    }
                    default: {
                        violations.push_back({.field_name = name, .message = "Field has unknown value type."});
                        return violations;
                    }
                }
            }

            for (const auto& missing_name: known_field_names) {
                violations.push_back({.field_name=missing_name, .message="Field is missing in metadata"});
            }
            return violations;
        }

        static std::unordered_map<std::string, MetadataFieldSchema*> ConvertToMetadataSchemaMap(const MetadataSchemaPtr& metadata_schema) {
            std::unordered_map<std::string, MetadataFieldSchema*> map;
            for (int i = 0; i < metadata_schema->fields_size(); ++i) {
                auto name = metadata_schema->fields(i).name();
                map[name] = metadata_schema->mutable_fields(i);
            }
            return map;
        }


//        static void AddStringMetadataField(Document& doc, std::string&name, )
    };
}

#endif //DOCUMENTUTILS_HPP
