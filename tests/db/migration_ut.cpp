#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <vector>
#include <string>
#include <format>

#include "db/migration_manager.hpp"
#include "env/env_manager.hpp"

using namespace bot;
using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Throw;

class MockEnvManager : public IEnvManager {
public:
    MOCK_METHOD(std::string, Get, (const std::string& key), (override));
};

class MockQueriesManager : public IQueriesManager {
public:
    MOCK_METHOD(std::string, Get, (const std::string& path), (override));
    MOCK_METHOD(std::vector<std::filesystem::path>, ListSubdirFiles, (const std::filesystem::path& subdir), (override));
};


class MigrationManagerTest : public ::testing::Test {
protected:
    std::shared_ptr<SQLite::Database> db_;
    std::shared_ptr<MockEnvManager> env_mock_;
    std::shared_ptr<MockQueriesManager> queries_mock_;

    const std::string kSqlDir_ = "sql";
    const std::string kMigrationsDir_ = "migrations";

    const std::string kCreateVerTable_ = "sql/create_version.sql";
    const std::string kGetVer_ = "sql/get_version.sql";
    const std::string kInsertVer_ = "sql/insert_version.sql";
    const std::string kCheckHash_ = "sql/check_hash.sql";

    void SetUp() override {
        db_ = std::make_shared<SQLite::Database>(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        env_mock_ = std::make_shared<NiceMock<MockEnvManager>>();
        queries_mock_ = std::make_shared<NiceMock<MockQueriesManager>>();

        SetupEnv();
        SetupSystemQueries();
    }

    void SetupEnv() {
        EXPECT_CALL(*env_mock_, Get("CREATE_VERSION_TABLE")).WillRepeatedly(Return(kCreateVerTable_));
        EXPECT_CALL(*env_mock_, Get("GET_CURRENT_VERSION")).WillRepeatedly(Return(kGetVer_));
        EXPECT_CALL(*env_mock_, Get("MIGRATIONS_DIR")).WillRepeatedly(Return(kMigrationsDir_));
        EXPECT_CALL(*env_mock_, Get("INSERT_VERSION_RECORD")).WillRepeatedly(Return(kInsertVer_));
        EXPECT_CALL(*env_mock_, Get("CHECK_MIGRATION_HASH")).WillRepeatedly(Return(kCheckHash_));
    }

    void SetupSystemQueries() {
        EXPECT_CALL(*queries_mock_, Get(kCreateVerTable_))
            .WillRepeatedly(Return(
                "CREATE TABLE IF NOT EXISTS schema_version (version INTEGER PRIMARY KEY, name TEXT, hash TEXT);"
            ));

        EXPECT_CALL(*queries_mock_, Get(kGetVer_))
            .WillRepeatedly(Return(
                "SELECT COALESCE(MAX(version), 0) FROM schema_version;"
            ));

        EXPECT_CALL(*queries_mock_, Get(kInsertVer_))
            .WillRepeatedly(Return(
                "INSERT INTO schema_version (version, name, hash) VALUES (?, ?, ?);"
            ));

        EXPECT_CALL(*queries_mock_, Get(kCheckHash_))
            .WillRepeatedly(Return(
                "SELECT COUNT(*) FROM schema_version WHERE version = ? AND name = ? AND hash = ?;"
            ));
    }

    std::string CalculateHash(const std::string& script) {
        std::hash<std::string> hasher;
        size_t hash_val = hasher(script);
        return std::format("{:016x}", hash_val);
    }

    void ManualInsertVersion(int version, const std::string& name, const std::string& script_content) {
        db_->exec("CREATE TABLE IF NOT EXISTS schema_version (version INTEGER PRIMARY KEY, name TEXT, hash TEXT);");

        std::string hash = CalculateHash(script_content);
        SQLite::Statement insert(*db_, "INSERT INTO schema_version VALUES (?, ?, ?)");
        insert.bind(1, version);
        insert.bind(2, name);
        insert.bind(3, hash);
        insert.exec();
    }
};

TEST_F(MigrationManagerTest, Run_FreshStart_AppliesMigration) {
    std::filesystem::path migration_file = "001_init_users.sql";
    std::string migration_sql = "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT);";

    EXPECT_CALL(*queries_mock_, ListSubdirFiles(std::filesystem::path(kMigrationsDir_)))
        .WillOnce(Return(std::vector<std::filesystem::path>{migration_file}));

    EXPECT_CALL(*queries_mock_, Get(kMigrationsDir_ + "/" + migration_file.string()))
        .WillOnce(Return(migration_sql));

    MigrationConfig config(env_mock_);
    MigrationManager manager(db_, queries_mock_, config);

    EXPECT_NO_THROW(manager.Run());

    EXPECT_TRUE(db_->tableExists("users"));

    int version = db_->execAndGet("SELECT MAX(version) FROM schema_version").getInt();
    EXPECT_EQ(version, 1);
}

TEST_F(MigrationManagerTest, Run_ExistingDb_SkipsOldAndAppliesNew) {
    std::string script_v1 = "CREATE TABLE users (id INTEGER);";
    ManualInsertVersion(1, "001_init.sql", script_v1);

    std::vector<std::filesystem::path> files = {"001_init.sql", "002_add_posts.sql"};
    std::string script_v2 = "CREATE TABLE posts (id INTEGER);";

    EXPECT_CALL(*queries_mock_, ListSubdirFiles(_))
        .WillOnce(Return(files));

    EXPECT_CALL(*queries_mock_, Get(kMigrationsDir_ + "/001_init.sql")).WillOnce(Return(script_v1));
    EXPECT_CALL(*queries_mock_, Get(kMigrationsDir_ + "/002_add_posts.sql")).WillOnce(Return(script_v2));

    MigrationConfig config(env_mock_);
    MigrationManager manager(db_, queries_mock_, config);
    manager.Run();

    EXPECT_TRUE(db_->tableExists("posts"));

    int version = db_->execAndGet("SELECT MAX(version) FROM schema_version").getInt();
    EXPECT_EQ(version, 2);
}

TEST_F(MigrationManagerTest, Run_DamagedMigration_ThrowsException) {
    ManualInsertVersion(1, "001_init.sql", "ORIGINAL SQL");

    EXPECT_CALL(*queries_mock_, ListSubdirFiles(_))
        .WillOnce(Return(std::vector<std::filesystem::path>{"001_init.sql"}));

    EXPECT_CALL(*queries_mock_, Get(kMigrationsDir_ + "/001_init.sql"))
        .WillOnce(Return("MODIFIED SQL"));

    MigrationConfig config(env_mock_);
    MigrationManager manager(db_, queries_mock_, config);

    EXPECT_THROW({
        try {
            manager.Run();
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ(e.what(), "Migration was damaged!");
            throw;
        }
    }, std::invalid_argument);
}

TEST_F(MigrationManagerTest, Run_BadSql_RollsBackTransaction) {
    std::filesystem::path bad_file = "005_bad.sql";
    std::string bad_sql = "CREATE TABLE broken_table (id INT; ==error== lim_(n->inf) tg(n)/n) ==error==";

    EXPECT_CALL(*queries_mock_, ListSubdirFiles(_))
        .WillOnce(Return(std::vector<std::filesystem::path>{bad_file}));

    EXPECT_CALL(*queries_mock_, Get(kMigrationsDir_ + "/005_bad.sql"))
        .WillOnce(Return(bad_sql));

    MigrationConfig config(env_mock_);
    MigrationManager manager(db_, queries_mock_, config);

    EXPECT_THROW(manager.Run(), SQLite::Exception);

    int count = db_->execAndGet("SELECT COUNT(*) FROM schema_version WHERE version = 5").getInt();
    EXPECT_EQ(count, 0);

    EXPECT_FALSE(db_->tableExists("broken_table"));
}
