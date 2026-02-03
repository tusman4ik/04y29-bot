#include "bootstrap.hpp"
#include "db/migration_manager.hpp"
#include "db/queries_manager.hpp"
#include "di/di.hpp"
#include "env/env_manager.hpp"
#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <tgbot/tgbot.h>

namespace bot {

void Bootstraper::Bootstrap() {
    StepOneLoggerSetup();
    StepTwoCheckAllTokens();
    StepThreeInitDatabase();
    StepFourInitTgBot();
    StepFiveLoadSqlScripts();
    StepSixRunMigrations();
    spdlog::info("Done");
}

void Bootstraper::StepOneLoggerSetup() {

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    console_sink->set_pattern("[%H:%M:%S] [%^%L%$]   %v");
    console_sink->set_color_mode(spdlog::color_mode::always);

    auto logger = std::make_shared<spdlog::logger>("default_logger", console_sink);

    spdlog::set_default_logger(logger);

    spdlog::set_level(spdlog::level::trace);
    spdlog::info("Bootstrap. Stage 1");
}

void Bootstraper::StepTwoCheckAllTokens() {
    spdlog::info("Bootstrap. Stage 2");
    REGISTER_I(ctx_, IEnvManager, EnvManager);
}

void Bootstraper::StepThreeInitDatabase() {
    spdlog::info("Bootstrap. Stage 3");
    REGISTER(ctx_, SQLite::Database, GET_ENV(ctx_, "DB_PATH"), SQLite::OPEN_READWRITE);
}

void Bootstraper::StepFourInitTgBot() {
    spdlog::info("Bootstrap. Stage 4");
    REGISTER(ctx_, TgBot::Bot, GET_ENV(ctx_, "BOT_TOKEN"));
}

void Bootstraper::StepFiveLoadSqlScripts() {
    spdlog::info("Bootstrap. Stage 5");
    REGISTER_I(ctx_, IQueriesManager, QueriesManager, GET_ENV(ctx_, "SQL_DIR"));
}

void Bootstraper::StepSixRunMigrations() {
    spdlog::info("Bootstrap. Stage 6");
    ApplyMigrations(ctx_);
}

}    // namespace bot
