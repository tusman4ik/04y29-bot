#pragma once

#include <string>
#include <unordered_map>

namespace bot {

class IEnvManager {
public:
    virtual std::string Get(const std::string& key);
    virtual ~IEnvManager() = default;
};

class EnvManager : public IEnvManager {
private:
    std::unordered_map<std::string, std::string> storage_;

public:
    EnvManager();

    std::string Get(const std::string& key) override;
};

}    // namespace bot
