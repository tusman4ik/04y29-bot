#pragma once

#include "db/queries_manager.hpp"
#include "di/di.hpp"
#include "env/env_manager.hpp"
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <filesystem>
#include <memory>
#include <vector>

namespace bot {

struct MigrationConfig {
    std::string create_version_table;
    std::string get_current_version;
    std::string migrations_dir;
    std::string insert_version_record;
    std::string check_migration_hash;
    explicit MigrationConfig(const std::shared_ptr<IEnvManager>& env_manager);
};

class MigrationManager {
private:
    std::shared_ptr<SQLite::Database> db_;
    std::shared_ptr<IQueriesManager> queries_manager_;
    MigrationConfig config_;

public:
    MigrationManager(const std::shared_ptr<SQLite::Database>& db,
                     const std::shared_ptr<IQueriesManager>& queries_manager,
                     const MigrationConfig& config);

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
