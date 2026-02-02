#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace bot {

class QueriesManager {
private:
    std::unordered_map<std::filesystem::path, std::string> storage_;

public:
    QueriesManager(const std::filesystem::path& sql_dir);

    std::string Get(const std::string& path);
    std::vector<std::filesystem::path> ListSubdirFiles(const std::filesystem::path& subdir);
};

}    // namespace bot
