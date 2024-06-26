//
// Created by RobinQu on 2024/4/24.
//

#ifndef DATABASEMIGRATION_HPP
#define DATABASEMIGRATION_HPP

#include <instinct/database/BaseConnectionPool.hpp>
#include <instinct/DataGlobals.hpp>
#include <instinct/database/IDatabaseMigration.hpp>

namespace INSTINCT_DATA_NS {

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
