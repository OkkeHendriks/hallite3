#pragma once

#include <string>

namespace hlt {
    namespace flog {
        void next_turn();
        void open(int bot_id);
        void flog(int x, int y, const std::string& message);
    }
}
