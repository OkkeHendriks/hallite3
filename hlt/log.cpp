#include "log.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

static std::ofstream log_file;
static std::vector<std::string> log_buffer;
static bool has_opened = false;
static bool has_atexit = false;
static auto milisStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

void dump_buffer_at_exit() {
    if (has_opened) {
        return;
    }

    auto nowInNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    std::string filename = "bot-unknown-" + std::to_string(nowInNanos) + ".log";
    std::ofstream file(filename, std::ios::trunc | std::ios::out);
    for (const std::string& message : log_buffer) {
        file << message << std::endl;
    }
}

void hlt::log::open(int bot_id) {
    if (has_opened) {
        hlt::log::log("Error: log: tried to open(" + std::to_string(bot_id) + ") but we have already opened before.");
        exit(1);
    }

    has_opened = true;
    std::string filename = "bot-" + std::to_string(bot_id) + ".log";
    log_file.open(filename, std::ios::trunc | std::ios::out);

    for (const std::string& message : log_buffer) {
        log_file << message << std::endl;
    }
    log_buffer.clear();
}

void hlt::log::log(const std::string& message) {
    auto milis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() - milisStart;
    if (has_opened) {
        log_file << std::to_string(milis) << "| " << message << std::endl;
    } else {
        if (!has_atexit) {
            has_atexit = true;
            atexit(dump_buffer_at_exit);
        }
        log_buffer.push_back(message);
    }
}
