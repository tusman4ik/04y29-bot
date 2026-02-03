#pragma once

#include "db/queries_manager.hpp"
#include "di/di.hpp"
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <filesystem>
#include <memory>
#include <vector>

namespace bot {

struct Config {
    std::string migrations_dir = "migrations";
    std::string internal_dir = "internal";
    std::string create_version_table = internal_dir +  "/create_version_table.sql";
    std::string get_current_version = internal_dir +  "/get_current_version.sql";
    std::string insert_version_record = internal_dir +  "/insert_version_record.sql";
    std::string check_migration_hash = internal_dir +  "/check_migration_hash.sql";
};

class MigrationManager {
private:
    std::shared_ptr<SQLite::Database> db_;
    std::shared_ptr<IQueriesManager> queries_manager_;
    Config config_;
public:
    MigrationManager(const std::shared_ptr<SQLite::Database>& db,
                     const std::shared_ptr<IQueriesManager>& queries_manager, const Config config_);
    void Run();

private:
    void EnsureVersionTable();
    int32_t GetCurrentVersion();
    std::vector<std::filesystem::path> GetMissingMigrations(int32_t version);

    void ValidateOldMigration(const std::filesystem::path& path, int32_t version,
                              const std::string& new_formated_hash);

    void RunMigrationScript(const std::string& script, const std::string& name,
                            int32_t version, const std::string& formated_hash);

    bool IsStartWithThreeDigits(const std::filesystem::path& path);
    int32_t GetVersion(const std::filesystem::path& path);
};

void ApplyMigrations(DiContainer& ctx);

}    // namespace bot
