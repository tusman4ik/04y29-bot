#pragma once

#include <curl/curl.h>
#include <string>

namespace bot {



class SheetsViewer {
private:
    const std::string api_key_;

public:
    SheetsViewer(const std::string& api_key);

    SheetsViewer& From(const std::string& table_id,
                       const std::string& page_name);
};

}    // namespace bot