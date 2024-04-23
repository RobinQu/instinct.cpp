//
// Created by RobinQu on 2024/4/21.
//

#ifndef IDATAMAPPER_HPP
#define IDATAMAPPER_HPP

#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    using SQLTemplate = std::string_view;
    using SQLContext = nlohmann::ordered_json;

    /**
     * Data mapper with SQL statements
     * @tparam T
     */
    template<
        typename T,
        typename PrimaryKey
    >
    class IDataMapper {
    public:
        IDataMapper()=default;
        virtual ~IDataMapper()=default;
        IDataMapper(IDataMapper&&)=delete;
        IDataMapper(const IDataMapper&)=delete;

        /**
         * Find one entity
         * @param select_sql
         * @param context
         * @return
         */
        virtual std::optional<T> SelectOne(const SQLTemplate& select_sql, const SQLContext& context) = 0;

        /**
         * Find many entities
         * @param select_sql
         * @param context
         * @return
         */
        virtual std::vector<T> SelectMany(const SQLTemplate& select_sql, const SQLContext& context) = 0;

        /**
         * For statements that return a number like insert/update/select count(*)
         * @param insert_sql
         * @param context
         * @return changed row count
         */
        virtual size_t Execute(const SQLTemplate& insert_sql, const SQLContext& context) = 0;

        virtual PrimaryKey InsertOne(const SQLTemplate& insert_sql, const SQLContext& context) = 0;

        virtual std::vector<PrimaryKey> InsertMany(const SQLTemplate& insert_sql, const SQLContext& context) = 0;
    };

    template<typename T>
    using DataMapperPtr = std::shared_ptr<IDataMapper<T>>;
}


#endif //IDATAMAPPER_HPP
