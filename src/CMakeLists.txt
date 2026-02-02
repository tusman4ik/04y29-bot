file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(bot ${SOURCES})

target_compile_definitions(bot PRIVATE SPDLOG_FMT_EXTERNAL)
target_link_libraries(
    bot
    PRIVATE
    fmt::fmt
    TgBot::TgBot
    SQLiteCpp
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    Threads::Threads
    ${SQLite3_LIBRARIES}
    nlohmann_json::nlohmann_json
)


include_directories(bot PRIVATE .)
target_include_directories(bot PRIVATE ${SQLite3_INCLUDE_DIRS})
