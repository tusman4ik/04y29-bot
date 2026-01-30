cmake_minimum_required(VERSION 3.10)
project(04y29-bot)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(SQLiteCpp REQUIRED) 
find_package(nlohmann_json REQUIRED)
find_package(TgBot REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_subdirectory(src)