#pragma once

#include <vector>
#include <queue>

#include "types.hpp"
#include "position.hpp"
#include "ship.hpp"
#include "dropoff.hpp"

namespace hlt {
    struct MapCell : public std::enable_shared_from_this<MapCell> {
        Position position;
        Halite halite;
        std::shared_ptr<Ship> ship;
        std::shared_ptr<Entity> structure; // only has dropoffs and shipyards; if id is -1, then it's a shipyard, otherwise it's a dropoff
        std::vector<std::shared_ptr<MapCell>> neighbourghs;
        std::queue<bool> occupiedAt;

        MapCell(int x, int y, Halite halite) :
            position(x, y),
            halite(halite)
        {}

        bool is_empty() const {
            return !ship && !structure;
        }

        bool is_occupied() const {
            return static_cast<bool>(ship);
        }

        bool has_structure() const {
            return static_cast<bool>(structure);
        }

        void mark_unsafe(const std::shared_ptr<Ship>& ship) {
            this->ship = ship;
        }

        std::shared_ptr<MapCell> directional_offset(Direction d) {
            auto dx = 0;
            auto dy = 0;
            switch (d) {
                case Direction::NORTH:
                    return neighbourghs.at(0);
                case Direction::EAST:
                    return neighbourghs.at(1);
                case Direction::SOUTH:
                    return neighbourghs.at(2);
                case Direction::WEST:
                    return neighbourghs.at(3);
                case Direction::STILL:
                    return shared_from_this();
                    break;
                default:
                    log::log(std::string("Error: MapCell directional_offset: unknown direction ") + static_cast<char>(d));
            }
        }

        void add_neighbourgh(const std::shared_ptr<MapCell>& neighbourgh)
        {
            neighbourghs.emplace_back(neighbourgh);
        }
    };
}
