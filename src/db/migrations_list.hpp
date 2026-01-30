#pragma once

#include <filesystem>
#include <vector>

namespace bot {

struct Migration {
    uint16_t version;
    std::filesystem::path path;
};

std::vector<Migration> migrations = {
    {1, "001_init_bd.sql"},
    
};

}