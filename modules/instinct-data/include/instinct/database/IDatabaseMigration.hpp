//
// Created by RobinQu on 2024/4/24.
//

#ifndef IDATABASEMIGRATION_HPP
#define IDATABASEMIGRATION_HPP

#include "DataGlobals.hpp"

namespace INSTINCT_DATA_NS {

    using DBSchemaVersion = unsigned int;

    class IDatabaseMigration {
    public:
        IDatabaseMigration()=default;
        virtual ~IDatabaseMigration()=default;
        IDatabaseMigration(const IDatabaseMigration&)=delete;
        IDatabaseMigration(IDatabaseMigration&&)=delete;


        /**
         * upgrade to latest
         * @return
         */
        virtual bool Migrate() = 0;

        /**
         * Rollback given steps
         * @param steps
         * @return
         */
        virtual bool Rollback(unsigned int steps) = 0;

        /**
         * Get current schema version
         * @return
         */
        virtual DBSchemaVersion GetVersion() = 0;


    };
}

#endif //IDATABASEMIGRATION_HPP
