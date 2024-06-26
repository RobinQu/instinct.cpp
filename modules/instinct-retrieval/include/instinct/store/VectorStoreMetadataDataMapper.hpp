//
// Created by RobinQu on 2024/5/27.
//

#ifndef VECTORSTOREINSTANCEMETADATAMAPPER_HPP
#define VECTORSTOREINSTANCEMETADATAMAPPER_HPP

#include <instinct/database/IDataTemplate.hpp>
#include <instinct/RetrievalGlobals.hpp>
#include <instinct/database/duckdb/DuckDBConnectionPool.hpp>
#include <instinct/database/duckdb/DuckDBDataTemplate.hpp>

namespace INSTINCT_RETRIEVAL_NS {


    using namespace INSTINCT_DATA_NS;
    class VectorStoreMetadataDataMapper {
        DataTemplatePtr<VectorStoreInstanceMetadata, std::string> date_template_;
    public:
        explicit VectorStoreMetadataDataMapper(DataTemplatePtr<VectorStoreInstanceMetadata, std::string> date_template)
            : date_template_(std::move(date_template)) {
        }

        [[nodiscard]] std::optional<std::string> InsertInstance(const VectorStoreInstanceMetadata& insert) const {
            SQLContext context;
            ProtobufUtils::ConvertMessageToJsonObject(insert, context);
            return date_template_->InsertOne(R"(
insert into instinct_vector_store_metadata(instance_id, metadata_schema, embedding_table_name, custom) values(
    {{text(instance_id)}},
    {{stringify(metadata_schema)}},
    {{text(embedding_table_name)}},
    {% if exists("custom") %}
    {{stringify(custom)}}
    {% else %}
    NULL
    {% endif %}
);
)", context);
        }

        [[nodiscard]] std::vector<VectorStoreInstanceMetadata> ListInstances() const {
            SQLContext context;
            return date_template_->SelectMany(R"(select * from instinct_vector_store_metadata)", context);
        }

        [[nodiscard]] size_t RemoveInstance(const std::string& instance_id) const {
            SQLContext context;
            context["instance_id"] = instance_id;
            return date_template_->Execute(R"(delete from instinct_vector_store_metadata where instance_id={{text(instance_id)}})", context);
        }

        [[nodiscard]] std::optional<VectorStoreInstanceMetadata> GetInstance(const std::string& instance_id) const {
            SQLContext context;
            context["instance_id"] = instance_id;
            return date_template_->SelectOne(R"(select * from instinct_vector_store_metadata where instance_id={{text(instance_id)}};)", context);
        }
    };

    using VectorStoreMetadataDataMapperPtr = std::shared_ptr<VectorStoreMetadataDataMapper>;

}



#endif //VECTORSTOREINSTANCEMETADATAMAPPER_HPP
