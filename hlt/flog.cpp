#include "flog.hpp"
#include "log.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

static std::ofstream flog_file;
static std::vector<std::string> flog_buffer;
static bool flog_has_opened = false;
static bool flog_has_atexit = false;
static int flog_turn = 0;

void hlt::flog::next_turn() {
    flog_turn++;
}

void flog_dump_buffer_at_exit() {
    if (flog_has_opened) {
        flog_file << "]" << std::endl;
        return;
    }

    auto now_in_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    std::string filename = "bot-unknown-" + std::to_string(now_in_nanos) + ".flog";
    std::ofstream file(filename, std::ios::trunc | std::ios::out);
    file << "[" << std::endl;
    for (const std::string& message : flog_buffer) {
        file << message << std::endl;
    }
    file << "]" << std::endl;
}

void hlt::flog::open(int bot_id) {
    if (flog_has_opened) {
        hlt::log::log("Error: flog: tried to open(" + std::to_string(bot_id) + ") but we have already opened before.");
        exit(1);
    }

    flog_has_opened = true;
    std::string filename = "bot-" + std::to_string(bot_id) + ".flog";
    flog_file.open(filename, std::ios::trunc | std::ios::out);

    flog_file << "[" << std::endl;
    for (const std::string& message : flog_buffer) {
        flog_file << message << std::endl;
    }
    flog_buffer.clear();
}

void hlt::flog::flog(int x, int y, const std::string& message) {
    if (flog_has_opened) {
        flog_file << "{\"t\":" << flog_turn << ", \"x\":" << x << ", \"y\":" << y <<  ", \"msg\":\"" << message << "\"}," << std::endl;
    } else {
        if (!flog_has_atexit) {
            flog_has_atexit = true;
            atexit(flog_dump_buffer_at_exit);
        }
        flog_buffer.push_back(message);
    }
}
