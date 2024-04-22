//
// Created by RobinQu on 2024/4/21.
//

#ifndef IDATAMAPPER_HPP
#define IDATAMAPPER_HPP

#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    template<
        typename T,
        typename SQL = std::string
    >
    class IDataMapper {
    public:
        IDataMapper()=default;
        virtual ~IDataMapper()=default;
        IDataMapper(IDataMapper&&)=delete;
        IDataMapper(const IDataMapper&)=delete;

        virtual T SelectOne(const SQL& select_sql) = 0;
        virtual std::vector<T> SelectMany(const SQL& select_sql) = 0;
        virtual int Update(const SQL& update_sql) = 0;
        virtual int Delete(const SQL& delete_sql) = 0;

    };
}


#endif //IDATAMAPPER_HPP
