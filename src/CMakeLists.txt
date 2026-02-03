include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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

include_directories(bot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(bot PRIVATE ${SQLite3_INCLUDE_DIRS})

# ===============================================================
# ========================= ( TEST LIB) =========================
# ===============================================================

set(TESTABLE_SOURCES
    db/migration_manager.cpp
    db/queries_manager.cpp
    env/env_manager.cpp
)

add_library(test_objects STATIC ${TESTABLE_SOURCES})

target_include_directories(test_objects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(test_objects
    PUBLIC
    SQLiteCpp
    nlohmann_json::nlohmann_json
)
