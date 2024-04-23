//
// Created by RobinQu on 2024/4/22.
//

#ifndef ENTITYDATAMAPPER_HPP
#define ENTITYDATAMAPPER_HPP

#include <inja/inja.hpp>
#include <store/duckdb/BaseDuckDBStore.hpp>

#include "../BaseConnectionPool.hpp"
#include "../IDataMapper.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    template<typename Entity, typename PrimaryKey = std::string>
        requires IsProtobufMessage<Entity>
    class DuckDBDataMapper final : public IDataMapper<Entity, PrimaryKey> {
        ConnectionPoolPtr<duckdb::Connection> connection_pool_;
        std::shared_ptr<inja::Environment> env_;
        // from sql column name to entity field name
        std::unordered_map<std::string_view, std::string_view> column_names_mapping_;
    public:
        explicit DuckDBDataMapper(
            const ConnectionPoolPtr<duckdb::Connection> &connection_pool,
            const std::shared_ptr<inja::Environment>& env = DEFAULT_SQL_TEMPLATE_INJA_ENV,
            const std::unordered_map<std::string_view, std::string_view> &column_names_mapping = {})
            :   connection_pool_(connection_pool),
                env_(env),
                column_names_mapping_(column_names_mapping) {
        }

        std::optional<Entity> SelectOne(const SQLTemplate &select_sql, const SQLContext& context) override {
            const auto conn = connection_pool_->Acquire();
            DuckDBConnectionPool::GuardConnection {connection_pool_, conn};
            auto result = conn->GetImpl().Query(env_->render(select_sql, context));
            details::assert_query_ok(result);
            if (result->RowCount() == 0) {
                return {};
            }
            if (result->RowCount() > 1) {
                throw InstinctException("More than one rows are returned from SelectOne");
            }
            std::vector<Entity> result_vector;
            ConvertQueryResult_(result, result_vector);
            return result_vector.front();
        }

        std::vector<Entity> SelectMany(const SQLTemplate &select_sql, const SQLContext& context) override {
            const auto conn = connection_pool_->Acquire();
            DuckDBConnectionPool::GuardConnection {connection_pool_, conn};
            auto result = conn->GetImpl().Query(env_->render(select_sql, context));
            details::assert_query_ok(result);
            std::vector<Entity> result_vector;
            ConvertQueryResult_(result, result_vector);
            return result_vector;
        }

        size_t Execute(const SQLTemplate &sql, const SQLContext& context) override {
            const auto conn = connection_pool_->Acquire();
            DuckDBConnectionPool::GuardConnection {connection_pool_, conn};
            const auto result = conn->GetImpl().Query(env_->render(sql, context));
            details::assert_query_ok(result);
            return result->GetValue<uint64_t>(0,0);
        }

        PrimaryKey InsertOne(const SQLTemplate &insert_sql, const SQLContext &context) override {
            auto result = InsertMany(insert_sql, context);
            assert_true(result.size(), 1, "should have only one returned id");
            return result.front();
        }

        std::vector<PrimaryKey> InsertMany(const SQLTemplate &insert_sql, const SQLContext &context) override {
            const auto conn = connection_pool_->Acquire();
            DuckDBConnectionPool::GuardConnection {connection_pool_, conn};
            auto result = conn->GetImpl().Query(env_->render(insert_sql, context));
            details::assert_query_ok(result);
            std::vector<PrimaryKey> key_result;
            for(const auto& row: *result) {
                key_result.push_back(row.GetValue<PrimaryKey>());
            }
            return key_result;
        }

    private:
        void ConvertQueryResult_(const QueryResult &query_result, std::vector<Entity> &result) {
            for (const auto &row: query_result) {
                Entity entity;
                auto *descriptor = entity.GetDescriptor();
                auto *reflection = entity.GetReflection();
                for (int i = 0; i < result->names.size(); i++) {
                    auto name = result->names[i];
                    if(column_names_mapping_.contains(name)) { // apply mapping
                        name = column_names_mapping_[name];
                    }
                    auto *field_descriptor = descriptor->FindFieldByName(name);
                    assert_true(field_descriptor, "should have found field_ descriptor by column name: " + name);
                    switch (field_descriptor->cpp_type()) {
                        case FieldDescriptor::CPPTYPE_STRING: {
                            reflection->SetString(&entity, field_descriptor, row.GetValue<std::string>(i));
                            break;
                        }
                        case FieldDescriptor::CPPTYPE_INT32: {
                            reflection->SetInt32(&entity, field_descriptor, row.GetValue<int>(i));
                            break;
                        }
                        case FieldDescriptor::CPPTYPE_INT64: {
                            reflection->SetInt64(&entity, field_descriptor, row.GetValue<long>(i));
                            break;
                        }
                        case FieldDescriptor::CPPTYPE_UINT32: {
                            reflection->SetUInt32(&entity, field_descriptor, row.GetValue<u_int32_t>(i));
                            break;
                        }
                        case FieldDescriptor::CPPTYPE_UINT64: {
                            reflection->SetUInt64(&entity, field_descriptor, row.GetValue<u_int64_t>(i));
                            break;
                            }
                        case FieldDescriptor::CPPTYPE_DOUBLE: {
                            reflection->SetDouble(&entity, field_descriptor, row.GetValue<double>(i));
                            break;
                        }
                        case FieldDescriptor::CPPTYPE_FLOAT: {
                            reflection->SetFloat(&entity, field_descriptor, row.GetValue<float>(i));
                            break;
                        }
                        case FieldDescriptor::CPPTYPE_BOOL: {
                            reflection->SetBool(&entity, field_descriptor, row.GetValue<bool>(i));
                            break;
                        }
                        case FieldDescriptor::CPPTYPE_ENUM: {
                            auto enum_name = row.GetValue<std::string>(i);
                            auto enum_value_descriptor = field_descriptor->enum_type()->FindValueByName(enum_name);
                            assert_true(enum_value_descriptor, "should have found enum value by name " + enum_name);
                            reflection->SetEnum(&entity, field_descriptor, enum_value_descriptor);
                            break;
                        }
                        default: {
                            throw InstinctException(fmt::format("Unknown cpp_type for this field. name={}, cpp_type={}",
                                                                field_descriptor->name(),
                                                                field_descriptor->cpp_type_name()));
                        }
                    }
                }
                result.push_back(std::move(entity));
            }
        }
    };
}

#endif //ENTITYDATAMAPPER_HPP
