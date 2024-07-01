//
// Created by RobinQu on 2024/4/24.
//

#ifndef DATABASEMIGRATION_HPP
#define DATABASEMIGRATION_HPP

#include <instinct/database/base_connection_pool.hpp>
#include <instinct/data_global.hpp>
#include <instinct/database/database_migration.hpp>

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

    template<typename ConnecitonImpl, typename QueryResultImpl>
    class DatabaseMigration final: public IDatabaseMigration {
        std::filesystem::path migration_dir_;
        ConnectionPoolPtr<ConnecitonImpl, QueryResultImpl> connection_pool_;
    public:
        bool Migrate() override {

        }

        bool Rollback(unsigned steps) override {

        }

        DBSchemaVersion GetVersion() override {

        }
    };
}

#endif //DATABASEMIGRATION_HPP
