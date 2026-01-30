#include "migration_manager.hpp"
#include <filesystem>
#include <fstream>

namespace bot {

namespace {
std::string ReadFile(std::filesystem::path& path) {
    std::ifstream file(path);

    if (!file) {
        return "";   
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
}    // namespace

void ApplyMigrations(SQLite::Database& db) {}

}    // namespace bot