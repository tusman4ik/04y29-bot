#include "migration_manager.hpp"
#include "db/queries_manager.hpp"
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <cstdint>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace bot {

MigrationManager::MigrationManager(
    const std::shared_ptr<SQLite::Database>& db,
    const std::shared_ptr<IQueriesManager>& queries_manager, const Config config_)
    : db_(db), queries_manager_(queries_manager), config_(config_) {}

void ApplyMigrations(DiContainer& ctx) {
    MigrationManager(GET(ctx, SQLite::Database), GET(ctx, IQueriesManager), Config{})
        .Run();
}

void MigrationManager::Run() {
    EnsureVersionTable();
    int32_t current_version = GetCurrentVersion();

    std::vector<std::filesystem::path> migrations =
        queries_manager_->ListSubdirFiles(config_.migrations_dir);
    
    spdlog::debug("Database current version = {}. Migrations amount = {}", current_version, migrations.size());
    std::hash<std::string> hash;

    for (const auto& path : migrations) {
        std::string script = queries_manager_->Get(
            std::filesystem::path(config_.migrations_dir) / path.string());
        
        int32_t version = GetVersion(path);
        size_t script_hash = hash(script);
        std::string formated_hash = std::format("{:016x}", script_hash);

        if (version <= current_version) {
            ValidateOldMigration(path, version, formated_hash);
        } else {
            RunMigrationScript(script, path.string(), version, formated_hash);
        }
    }

    spdlog::debug("All migrations applied successfully");
}

void MigrationManager::RunMigrationScript(const std::string& script,
                                          const std::string& name, int32_t version,
                                          const std::string& formated_hash) {

    spdlog::debug("Apply migration = {} (version = {}, hash = {})", name, version,
                  formated_hash);
    SQLite::Transaction transaction(*db_);

    SQLite::Statement insert(*db_, queries_manager_->Get(config_.insert_version_record));

    insert.bind(1, version);
    insert.bind(2, name);
    insert.bind(3, formated_hash);

    insert.exec();
    db_->exec(script);

    transaction.commit();
}

void MigrationManager::ValidateOldMigration(const std::filesystem::path& path,
                                            int32_t version,
                                            const std::string& new_formated_hash) {
    spdlog::debug("Check migration = {} (version = {}, hash = {})", path.string(),
                  version, new_formated_hash);

    std::string script = queries_manager_->Get(config_.check_migration_hash);

    SQLite::Statement check(*db_, script);

    check.bind(1, version);
    check.bind(2, path.string());
    check.bind(3, new_formated_hash);

    if (check.executeStep() && check.getColumn(0).getInt() == 0) {
        throw std::invalid_argument("Migration was damaged!");
    }
}

void MigrationManager::EnsureVersionTable() {
    spdlog::info("Run init script = {}", config_.create_version_table);

    SQLite::Transaction transaction(*db_);
    db_->exec(queries_manager_->Get(config_.create_version_table));
    transaction.commit();
}

int32_t MigrationManager::GetCurrentVersion() {

    return db_->execAndGet(queries_manager_->Get(config_.get_current_version));
}

bool MigrationManager::IsStartWithThreeDigits(const std::filesystem::path& path) {
    std::string str = path.string();
    return str.size() >= 3 && std::isdigit(str[0]) && std::isdigit(str[1]) &&
           std::isdigit(str[2]);
}

int32_t MigrationManager::GetVersion(const std::filesystem::path& path) {
    std::string str = path.string();

    int8_t fst = str[0] - '0';
    int8_t scd = str[1] - '0';
    int8_t thd = str[2] - '0';

    return ((fst * 10) + scd) * 10 + thd;
}

}    // namespace bot
