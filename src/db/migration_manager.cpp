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

MigrationConfig::MigrationConfig(const std::shared_ptr<IEnvManager>& env_manager)
    : create_version_table(env_manager->Get("CREATE_VERSION_TABLE")),
      get_current_version(env_manager->Get("GET_CURRENT_VERSION")),
      migrations_dir(env_manager->Get("MIGRATIONS_DIR")),
      insert_version_record(env_manager->Get("INSERT_VERSION_RECORD")),
      check_migration_hash(env_manager->Get("CHECK_MIGRATION_HASH")) {}

MigrationManager::MigrationManager(const std::shared_ptr<SQLite::Database>& db,
                                   const std::shared_ptr<IQueriesManager>& queries_manager,
                                   const MigrationConfig& config)
    : db_(db), queries_manager_(queries_manager), config_(config) {}

void ApplyMigrations(DiContainer& ctx) {
    MigrationManager(GET(ctx, SQLite::Database), GET(ctx, IQueriesManager),
                     MigrationConfig(GET(ctx, IEnvManager)))
        .Run();
}

void MigrationManager::Run() {
    EnsureVersionTable();
    int32_t current_version = GetCurrentVersion();
    spdlog::debug("Database current version = {}", current_version);

    std::vector<std::filesystem::path> migrations =
        queries_manager_->ListSubdirFiles(config_.migrations_dir);
    ;

    std::hash<std::string> hash;

    for (const auto& path : migrations) {
        std::string script =
            queries_manager_->Get(config_.migrations_dir + '/' + path.string());
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

// std::vector<std::filesystem::path>
// MigrationManager::GetMissingMigrations(int32_t version) {

//     std::vector<std::filesystem::path> res;
//     std::vector<std::filesystem::path> all_scripts =
//         queries_manager_->ListSubdirFiles(config_.migrations_dir);

//     for (const auto& path : all_scripts) {
//         if (!IsStartWithThreeDigits(path) && GetCurrentVersion() > version) {
//             spdlog::error("Invalid transaction file = {}", path.string());
//             throw std::invalid_argument("Invalid transaction file");
//         }

//         res.push_back(path);
//     }
//     return res;
// }

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

// namespace {

// std::string ReadFile(std::filesystem::path& path) {
//     std::ifstream file(path);

//     if (!file) {
//         return "";
//     }

//     std::stringstream ss;
//     ss << file.rdbuf();
//     return ss.str();
// }

// bool StartWithThreeDigits(const std::filesystem::directory_entry& entry) {
//     std::string str = entry.path().string();
//     return str.size() >= 3 && std::isdigit(str[0]) && std::isdigit(str[1]) &&
//            std::isdigit(str[2]);
// }

// bool IsSqlFile(const std::filesystem::directory_entry& entry) {
//     return entry.is_regular_file() && entry.path().string().ends_with(".sql");
// }

// int8_t GetMigrationVersion(const std::filesystem::path& path) {
//     std::string str = path.string();

//     char fst = str[0] - '0';
//     char scn = str[1] - '0';
//     char thr = str[2] - '0';

//     return (fst * 10 + scn) * 10 + thr;
// }

// void RunInitScript(DiContainer& ctx, const std::shared_ptr<SQLite::Database>& db,
//                    const std::shared_ptr<QueriesManager>& queries_manager) {

//     std::string init_script_path = GET_ENV(ctx, "CREATE_VERSION_TABLE");

//     spdlog::info("Run init script = {}", init_script_path);

//     db->exec(queries_manager->Get(init_script_path));
// }

// void GetLastVersion() {

// }

// }    // namespace

// }

}    // namespace bot
