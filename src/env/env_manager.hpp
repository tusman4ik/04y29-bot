#pragma once

#include <string>
#include <unordered_map>

namespace bot {

class IEnvManager {
public:
    virtual std::string Get(const std::string& key) = 0;
    virtual ~IEnvManager() = default;
};

class EnvManager final : public IEnvManager {
private:
    std::unordered_map<std::string, std::string> storage_;

public:
    EnvManager();

    std::string Get(const std::string& key) override;
};

}    // namespace bot
