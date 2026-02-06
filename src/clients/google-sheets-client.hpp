#pragma once

#include <string>
#include <sys/stat.h>

namespace bot {

static const int kDefaultTimeout = 10;

struct RequestParams {
    std::string sheet_id;
    std::string list_name;
    std::string first_idx;    ///< 'name' of first column. For instance 'A', 'AM', etc
    std::string last_idx;
};

class IGoogleSheetsClient {
public:
    virtual std::string Pull(const RequestParams& params) const = 0;
    virtual ~IGoogleSheetsClient() = default;
};

class GoogleSheetsClient : public IGoogleSheetsClient {
private:
    static constexpr const char* kRowUrl =
        "https://sheets.googleapis.com/v4/spreadsheets/{}/values/{}?key={}";
    std::string api_key_;

public:
    GoogleSheetsClient(const std::string& api_key) : api_key_(api_key) {}

    std::string Pull(const RequestParams& params) const override;

private:
    std::string GetUrl(const RequestParams& params, const std::string& range) const;
    static std::string Perform(const std::string& url);
    static std::string GetRange(const RequestParams& params);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                                std::string* userp);
    void LogUrl(const std::string& url)const;
};

}    // namespace bot
