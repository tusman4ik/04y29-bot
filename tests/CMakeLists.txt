include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)

enable_testing()

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS "*.cpp")

add_executable(ut ${TEST_FILES})

target_link_libraries(ut

    test_objects
    nlohmann_json::nlohmann_json
    GTest::gtest_main
    GTest::gmock
    SQLiteCpp
    spdlog
)

include(GoogleTest)
gtest_discover_tests(ut)
