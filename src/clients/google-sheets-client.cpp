#include "google-sheets-client.hpp"
#include <curl/curl.h>
#include <curl/urlapi.h>
#include <format>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace bot {

std::string GoogleSheetsClient::Pull(const RequestParams& params) const {
    std::string range = GetRange(params);

    std::string url = GetUrl(params, range);

    LogUrl(url);

    return Perform(url);
}

std::string GoogleSheetsClient::Perform(const std::string& url) {

    std::unique_ptr<CURL, void (*)(CURL*)> curl(curl_easy_init(), curl_easy_cleanup);
    CURLcode res;
    std::string buffer;

    if (curl) {
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, kDefaultTimeout);
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, kDefaultTimeout);
        curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);

        res = curl_easy_perform(curl.get());

        if (res != CURLE_OK) {
            spdlog::error("CURL transport error: {}", curl_easy_strerror(res));
            throw std::runtime_error("Network error while calling Google API");
        }

        long http_code = 0;
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code != 200) {
            spdlog::warn("Google API returned error code {}. Body: \n{}", http_code,
                         buffer);
            throw std::runtime_error("Google Sheets API logical error");
        }
    }
    return buffer;
}

std::string GoogleSheetsClient::GetUrl(const RequestParams& params,
                                       const std::string& range) const {

    return std::format(kRowUrl, params.sheet_id, range, api_key_);
}

void GoogleSheetsClient::LogUrl(const std::string& url) const {
    std::string masked_url = url;
    size_t key_pos = masked_url.find("key=");
    if (key_pos != std::string::npos) {
        masked_url.replace(key_pos + 4, std::string::npos,
                           std::string('*', api_key_.size()));
    }
    spdlog::trace("Sending request to: {}", masked_url);
}

std::string GoogleSheetsClient::GetRange(const RequestParams& params) {
    return std::format("{}!{}:{}", params.list_name, params.first_idx, params.last_idx);
}

size_t GoogleSheetsClient::WriteCallback(void* contents, size_t size, size_t nmemb,
                                         std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

}    // namespace bot
