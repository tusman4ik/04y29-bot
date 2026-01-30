add_executable(
    bot

    main.cpp
    db/migration_manager.cpp
)

target_link_libraries(
    bot
    PRIVATE
    TgBot::TgBot
    SQLiteCpp
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    Threads::Threads
    ${SQLite3_LIBRARIES}  
    nlohmann_json::nlohmann_json
)

target_include_directories(bot PRIVATE ${SQLite3_INCLUDE_DIRS})