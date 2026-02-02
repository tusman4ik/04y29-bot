#include "env_manager.hpp"
#include "env.hpp"
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <unordered_map>

namespace bot {

EnvManager::EnvManager() {

    for (auto [key, is_opt, default_val] : tokens) {
        const char* val = std::getenv(key);
        if (val) {
            storage_[key] = val;
            continue;
        }

        if (!is_opt) {
            throw std::runtime_error("Required env = " + std::string(key));
        }

        if (default_val) {
            storage_[key] = default_val;
        }
    }
}

std::string EnvManager::Get(const std::string& key) {
    if (!storage_.contains(key)) {
        throw std::runtime_error("Unknown env = " + key);
    }
    return storage_[key];
}

}    // namespace bot
