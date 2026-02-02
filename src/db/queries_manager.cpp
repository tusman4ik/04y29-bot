#include "queries_manager.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>

namespace bot {

namespace {

std::string ReadFile(const std::filesystem::path& path) {
    std::ifstream file(path);

    if (!file) {
        return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool IsSqlFile(const std::filesystem::directory_entry& entry) {
    return entry.is_regular_file() && entry.path().string().ends_with(".sql");
}

bool IsParent(const std::filesystem::path& parent, const std::filesystem::path& child) {
    auto [p_it, c_it] =
        std::mismatch(parent.begin(), parent.end(), child.begin(), child.end());

    return p_it == parent.end() && c_it != child.end();
}

}    // namespace

QueriesManager::QueriesManager(const std::filesystem::path& sql_dir) {
    for (const auto& it : std::filesystem::recursive_directory_iterator(sql_dir)) {
        if (!IsSqlFile(it)) {
            continue;
        }

        std::filesystem::path full_path = it.path();
        std::filesystem::path relative_path = full_path.lexically_relative(sql_dir);

        storage_[relative_path.string()] = ReadFile(full_path);
        spdlog::debug("Found sql script = {}", relative_path.string());
    }
}

std::string QueriesManager::Get(const std::string& path) {
    auto it = storage_.find(path);
    if (it == storage_.end()) {
        throw std::runtime_error("Sql script = " + path + " not found");
    }
    return it->second;
}

std::vector<std::filesystem::path>
QueriesManager::ListSubdirFiles(const std::filesystem::path& subdir) {
    std::vector<std::filesystem::path> res;
    for (const auto& it : storage_) {
        if (IsParent(subdir, it.first)) {
            res.push_back(it.first.lexically_relative(subdir));
        }
    }

    std::sort(res.begin(), res.end());
    return res;
}

}    // namespace bot
