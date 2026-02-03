#pragma once

namespace bot {

struct Env {
    Env(const char* key, bool is_optional, const char* default_value = nullptr)
        : key(key), is_optional(is_optional), default_value(default_value) {}

    const char* key;
    bool is_optional;
    const char* default_value;
};

Env tokens[] = {{"BOT_TOKEN", false},
                {"DB_PATH", true, "/app/data/data.db"},
                {"GOOGLE_SHEETS_API_KEY", false},
                {"SQL_DIR", true, "sql"},
                {"MIGRATIONS_DIR", true, "migrations"},
                {"CREATE_VERSION_TABLE", true, "internal/create_version_table.sql"},
                {"GET_CURRENT_VERSION", true, "internal/get_current_version.sql"},
                {"INSERT_VERSION_RECORD", true, "internal/insert_version_record.sql"},
                {"CHECK_MIGRATION_HASH", true, "internal/check_migration_hash.sql"}

};

}    // namespace bot
