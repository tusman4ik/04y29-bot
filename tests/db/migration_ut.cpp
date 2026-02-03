#include <SQLiteCpp/SQLiteCpp.h>
#include <filesystem>
#include <format>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "db/migration_manager.hpp"

using namespace bot;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

class MockQueriesManager : public IQueriesManager {
public:
    MOCK_METHOD(std::string, Get, (const std::string& path), (override));
    MOCK_METHOD(std::vector<std::filesystem::path>, ListSubdirFiles,
                (const std::filesystem::path& subdir), (override));
};

class MigrationManagerTest : public ::testing::Test {
protected:
    std::shared_ptr<SQLite::Database> db_;
    std::shared_ptr<MockQueriesManager> queries_mock_;
    Config test_config_;

    const std::string kMigDir_ = "test_migrations";
    const std::string kSysDir_ = "test_internal";

    void SetUp() override {
        db_ = std::make_shared<SQLite::Database>(":memory:", SQLite::OPEN_READWRITE |
                                                                 SQLite::OPEN_CREATE);
        queries_mock_ = std::make_shared<NiceMock<MockQueriesManager>>();

        test_config_.migrations_dir = kMigDir_ + "/";
        test_config_.internal_dir = kSysDir_ + "/";
        test_config_.create_version_table = kSysDir_ + "/create.sql";
        test_config_.get_current_version = kSysDir_ + "/get.sql";
        test_config_.insert_version_record = kSysDir_ + "/insert.sql";
        test_config_.check_migration_hash = kSysDir_ + "/check.sql";

        SetupSystemQueries();
    }
    void SetupSystemQueries() {
        EXPECT_CALL(*queries_mock_, Get(test_config_.create_version_table))
            .WillRepeatedly(Return("CREATE TABLE IF NOT EXISTS schema_version (version "
                                   "INTEGER PRIMARY KEY, name TEXT, hash TEXT);"));

        EXPECT_CALL(*queries_mock_, Get(test_config_.get_current_version))
            .WillRepeatedly(
                Return("SELECT COALESCE(MAX(version), 0) FROM schema_version;"));

        EXPECT_CALL(*queries_mock_, Get(test_config_.insert_version_record))
            .WillRepeatedly(Return(
                "INSERT INTO schema_version (version, name, hash) VALUES (?, ?, ?);"));

        EXPECT_CALL(*queries_mock_, Get(test_config_.check_migration_hash))
            .WillRepeatedly(Return("SELECT COUNT(*) FROM schema_version WHERE version = "
                                   "? AND name = ? AND hash = ?;"));
    }

    std::string CalculateHash(const std::string& script) {
        std::hash<std::string> hasher;
        return std::format("{:016x}", hasher(script));
    }

    void ManualInsertVersion(int version, const std::string& name,
                             const std::string& script_content) {
        db_->exec("CREATE TABLE IF NOT EXISTS schema_version (version INTEGER PRIMARY "
                  "KEY, name TEXT, hash TEXT);");
        std::string hash = CalculateHash(script_content);
        SQLite::Statement insert(*db_, "INSERT INTO schema_version VALUES (?, ?, ?)");
        insert.bind(1, version);
        insert.bind(2, name);
        insert.bind(3, hash);
        insert.exec();
    }
};

TEST_F(MigrationManagerTest, Run_FreshStart_AppliesMigration) {
    std::filesystem::path migration_file = "001_init.sql";
    std::string migration_sql = "CREATE TABLE users (id INTEGER PRIMARY KEY);";

    EXPECT_CALL(*queries_mock_,
                ListSubdirFiles(std::filesystem::path(test_config_.migrations_dir)))
        .WillOnce(Return(std::vector<std::filesystem::path>{migration_file}));

    EXPECT_CALL(*queries_mock_,
                Get(test_config_.migrations_dir + migration_file.string()))
        .WillOnce(Return(migration_sql));

    MigrationManager manager(db_, queries_mock_, test_config_);

    EXPECT_NO_THROW(manager.Run());
    EXPECT_TRUE(db_->tableExists("users"));
    EXPECT_EQ(db_->execAndGet("SELECT MAX(version) FROM schema_version").getInt(), 1);
}

TEST_F(MigrationManagerTest, Run_ExistingDb_SkipsOldAndAppliesNew) {
    std::string script_v1 = "CREATE TABLE users (id INTEGER);";
    ManualInsertVersion(1, "001_init.sql", script_v1);

    std::vector<std::filesystem::path> files = {"001_init.sql", "002_add_posts.sql"};
    std::string script_v2 = "CREATE TABLE posts (id INTEGER);";

    EXPECT_CALL(*queries_mock_, ListSubdirFiles(_)).WillOnce(Return(files));
    EXPECT_CALL(*queries_mock_, Get(test_config_.migrations_dir + "001_init.sql"))
        .WillOnce(Return(script_v1));
    EXPECT_CALL(*queries_mock_, Get(test_config_.migrations_dir + "002_add_posts.sql"))
        .WillOnce(Return(script_v2));

    MigrationManager manager(db_, queries_mock_, test_config_);
    manager.Run();

    EXPECT_TRUE(db_->tableExists("posts"));
    EXPECT_EQ(db_->execAndGet("SELECT MAX(version) FROM schema_version").getInt(), 2);
}

TEST_F(MigrationManagerTest, Run_DamagedMigration_ThrowsException) {
    ManualInsertVersion(1, "001_init.sql", "ORIGINAL SQL");

    EXPECT_CALL(*queries_mock_, ListSubdirFiles(_))
        .WillOnce(Return(std::vector<std::filesystem::path>{"001_init.sql"}));

    EXPECT_CALL(*queries_mock_, Get(test_config_.migrations_dir + "001_init.sql"))
        .WillOnce(Return("MODIFIED SQL"));

    MigrationManager manager(db_, queries_mock_, test_config_);

    EXPECT_THROW(manager.Run(), std::invalid_argument);
}

TEST_F(MigrationManagerTest, Run_BadSql_RollsBackTransaction) {
    std::filesystem::path bad_file = "005_bad.sql";
    std::string bad_sql = "CREATE TABLE broken_table (id INT); syntax_error_here;";

    EXPECT_CALL(*queries_mock_, ListSubdirFiles(_))
        .WillOnce(Return(std::vector<std::filesystem::path>{bad_file}));
    EXPECT_CALL(*queries_mock_, Get(test_config_.migrations_dir + bad_file.string()))
        .WillOnce(Return(bad_sql));

    MigrationManager manager(db_, queries_mock_, test_config_);

    EXPECT_THROW(manager.Run(), SQLite::Exception);
    EXPECT_FALSE(db_->tableExists("broken_table"));
}
