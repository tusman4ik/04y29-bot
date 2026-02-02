#pragma once

#include "di/di.hpp"
#include <SQLiteCpp/Database.h>
#include <tgbot/Bot.h>

namespace bot {

class Bootstraper {
private:
    DiContainer ctx_;

public:
    void Bootstrap();

private:
    void StepOneLoggerSetup();
    void StepTwoCheckAllTokens();
    void StepThreeInitDatabase();
    void StepFourInitTgBot();
    void StepFiveLoadSqlScripts();
    void StepSixRunMigrations();
    // void StepFiveInitSheetsViewer();
    // void Step
    // void StepSevenInitDaoLayer();
};

}    // namespace bot
