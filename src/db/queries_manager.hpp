#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace bot {

class IQueriesManager {
public:
    virtual std::string Get(const std::string& path) = 0;

    virtual std::vector<std::filesystem::path>
    ListSubdirFiles(const std::filesystem::path& subdir) = 0;

    virtual ~IQueriesManager() = default;
};

class QueriesManager : public IQueriesManager {
private:
    std::unordered_map<std::filesystem::path, std::string> storage_;

public:
    QueriesManager(const std::filesystem::path& sql_dir);

    std::string Get(const std::string& path) override;

    std::vector<std::filesystem::path>
    ListSubdirFiles(const std::filesystem::path& subdir) override;
};

}    // namespace bot
