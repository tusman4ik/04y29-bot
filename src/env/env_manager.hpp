#pragma once

#include <string>
#include <unordered_map>

namespace bot {

class EnvManager {
private:
    std::unordered_map<std::string, std::string> storage_;

public:
    EnvManager();
    
    std::string Get(const std::string& key);
};

}    // namespace bot