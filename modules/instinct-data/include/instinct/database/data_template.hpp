//
// Created by RobinQu on 2024/4/21.
//

#ifndef IDATAMAPPER_HPP
#define IDATAMAPPER_HPP

#include <instinct/data_global.hpp>

namespace INSTINCT_DATA_NS {


    /**
     * Data mapper with SQL statements
     * @tparam T
     */
    template<
        typename T,
        typename PrimaryKey
    >
    class IDataTemplate {
    public:
        IDataTemplate()=default;
        virtual ~IDataTemplate()=default;
        IDataTemplate(IDataTemplate&&)=delete;
        IDataTemplate(const IDataTemplate&)=delete;

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

        /**
         * Execute sql for inserting one record
         * @param insert_sql should contain `retuning(id)` clause
         * @param context
         * @return
         */
        virtual PrimaryKey InsertOne(const SQLTemplate& insert_sql, const SQLContext& context) = 0;

        /**
         * Execute sql for inserting many records
         * @param insert_sql should contain `retuning(id)` clause
         * @param context 
         * @return 
         */
        virtual std::vector<PrimaryKey> InsertMany(const SQLTemplate& insert_sql, const SQLContext& context) = 0;

        /**
         * Run aggregation query
         * @param select_sql
         * @param context
         * @return
         */
        virtual Aggregations Aggregate(const SQLTemplate &select_sql, const SQLContext& context) = 0;
    };

    template<typename T, typename PrimaryKey>
    using DataTemplatePtr = std::shared_ptr<IDataTemplate<T,PrimaryKey>>;
}


#endif //IDATAMAPPER_HPP
