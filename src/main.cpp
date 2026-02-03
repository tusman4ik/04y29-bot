#include "bootstrap/bootstrap.hpp"

int main() {
    try {
        bot::Bootstraper().Bootstrap();
    } catch (std::exception& ex) {
        spdlog::error("Failed to run init script = {}", ex.what());
    }
    return 0;
}
