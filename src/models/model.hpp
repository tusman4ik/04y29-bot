#pragma once

#include <cstdint>
#include <string>

namespace bot {

struct User {
    uint16_t id;
    uint32_t isu;
    std::string tg_id;
    std::string name;
    std::string last_name;
    std::string father_name;
};

enum class SourceType { kGoogleSheet, kMyItmoPortal };

struct Source {
    uint16_t id;
    SourceType source_type;
    uint16_t update_frequency_in_min;
};

struct SourceMetaInf {
    uint16_t id;
    uint16_t source_id;
    std::string key;
    std::string value;
};

enum class Subject {
    kDmt, /* discrete math */
    kSdt, /* standard development tools */
    kFop, /* fundamental of programming */
    kMta, /* mathematical analysis */
    kLag, /* linear algebra */
    kAsd, /* algorithm and data structure */
    kPhe, /* physical education */
};

struct Score {
    uint16_t id;
    uint16_t user_id;
    Subject subject;
    double cnt;
    std::string name;
};

}    // namespace bot