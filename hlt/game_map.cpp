#include "game_map.hpp"
#include "input.hpp"
#include "flog.hpp"

void hlt::GameMap::_update() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            cells[y][x]->ship.reset();
        }
    }

    int update_count;
    hlt::get_sstream() >> update_count;
    log::log(std::to_string(update_count) + " cell updates received.");
    for (int i = 0; i < update_count; ++i) {
        int x;
        int y;
        int halite;
        hlt::get_sstream() >> x >> y >> halite;
        totalHallite += halite - cells[y][x]->halite;
        cells[y][x]->halite = halite; 
         
    }
    averageHallite = totalHallite / nrCells;
}

std::unique_ptr<hlt::GameMap> hlt::GameMap::_generate() {
    std::unique_ptr<hlt::GameMap> map = std::make_unique<GameMap>();

    hlt::get_sstream() >> map->width >> map->height;
    map->nrCells = map->width * map->height;

    map->cells.resize((size_t)map->height);
    for (int y = 0; y < map->height; ++y) {
        auto in = hlt::get_sstream();

        map->cells[y].reserve((size_t)map->width);
        for (int x = 0; x < map->width; ++x) {
            hlt::Halite halite;
            in >> halite;

            map->cells[y].push_back(std::make_shared<MapCell>(x, y, halite));
            map->totalHallite += halite;
        }
    }
    map->averageHallite = map->totalHallite / map->nrCells;

    log::log("Initializing neighbourgh lists");
    Position normalizePos;
    for (int y = 0; y < map->height; ++y) {
        for (int x = 0; x < map->width; ++x) {
            // North
            normalizePos.x = x - 1;
            normalizePos.y = y + 0;
            map->normalize(normalizePos);
            flog::flog(x, y, std::to_string(normalizePos.x) + ", " + std::to_string(normalizePos.y) );
            map->cells[y][x]->add_neighbourgh(map->cells[normalizePos.y][normalizePos.x]);

            // East
            normalizePos.x = x + 0;
            normalizePos.y = y + 1;
            map->normalize(normalizePos);
            flog::flog(x, y, std::to_string(normalizePos.x) + ", " + std::to_string(normalizePos.y) );
            map->cells[y][x]->add_neighbourgh(map->cells[normalizePos.y][normalizePos.x]);

            // South
            normalizePos.x = x + 1;
            normalizePos.y = y + 0;
            map->normalize(normalizePos);
            flog::flog(x, y, std::to_string(normalizePos.x) + ", " + std::to_string(normalizePos.y) );
            map->cells[y][x]->add_neighbourgh(map->cells[normalizePos.y][normalizePos.x]);

            // West
            normalizePos.x = x + 0;
            normalizePos.y = y - 1;
            map->normalize(normalizePos);
            flog::flog(x, y, std::to_string(normalizePos.x) + ", " + std::to_string(normalizePos.y) );
            map->cells[y][x]->add_neighbourgh(map->cells[normalizePos.y][normalizePos.x]);
        }
    }
    log::log("Neighbourgh lists initialized");

    return map;
}
