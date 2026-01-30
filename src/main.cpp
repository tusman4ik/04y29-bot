#include "sheets_api/sheets_viewer.hpp"
#include <SQLiteCpp/SQLiteCpp.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
#include <tgbot/tgbot.h>


std::string IsFound(const char* var) { return var ? "found" : "not found"; }

void LoggerSetup() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    console_sink->set_pattern("[%H:%M:%S] [%^%L%$]   %v");
    console_sink->set_color_mode(spdlog::color_mode::always);

    auto logger =
        std::make_shared<spdlog::logger>("default_logger", console_sink);

    spdlog::set_default_logger(logger);

    spdlog::set_level(spdlog::level::debug);
}

int main() {
    LoggerSetup();

    const char* tg_token = std::getenv("BOT_TOKEN");
    const char* db_path = std::getenv("DB_PATH");
    const char* google_sheets_api_key = std::getenv("GOOGLE_SHEETS_API_KEY");

    if (!tg_token || !db_path || !google_sheets_api_key) {
        spdlog::critical("Not found env variables: tg_token: {}, db_path: {}, "
                         "google_sheets_api_key",
                         IsFound(tg_token), IsFound(db_path),
                         IsFound(google_sheets_api_key));
        return 1;
    }
    spdlog::info("All env variables exist");

    TgBot::Bot bot(tg_token);

    try {
        SQLite::Database db(db_path, SQLite::OPEN_READWRITE);
    } catch (std::exception& e) {
        spdlog::critical("Database error: {}", e.what());
    }

    bot::SheetsViewer viewer(google_sheets_api_key);
    return 0;
}