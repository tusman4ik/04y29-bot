#pragma once

namespace bot {

struct Env {
    Env(const char* key, bool is_optional, const char* default_value = nullptr)
        : key(key), is_optional(is_optional), default_value(default_value) {}

    const char* key;
    bool is_optional;
    const char* default_value;
};

Env tokens[] = {
    {"BOT_TOKEN", false},
    {"DB_PATH", true, "/app/data/data.db"},
    {"GOOGLE_SHEETS_API_KEY", false},
    {"SQL_DIR", true, "sql"},
};

}    // namespace bot
