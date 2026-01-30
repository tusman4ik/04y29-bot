#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

namespace bot {

void ApplyMigrations(SQLite::Database& db);

}    // namespace bot