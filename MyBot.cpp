
#include "commander.h"

#ifndef BOT_NAME
#define BOT_NAME "unnamed_bot"
#endif

int main(int argc, char *argv[])
{
    unsigned int seed;
    if (argc > 1)
        seed = static_cast<unsigned int>(std::stoul(argv[1]));
    else
        seed = static_cast<unsigned int>(time(nullptr));

    Commander commander(seed, BOT_NAME);
    
    while(commander.turn());
    log::log(std::string(BOT_NAME) + " done.");
    return 0;
}
