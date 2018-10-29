#pragma once

#include "types.hpp"
#include "map_cell.hpp"

#include <vector>
#include <algorithm>

namespace hlt
{
struct GameMap
{
    int width;
    int height;
    int nrCells;
    Halite averageHallite;
    Halite totalHallite;
    std::vector<std::vector<std::shared_ptr<MapCell>>> cells;

    std::shared_ptr<MapCell> at(const Position &position)
    {
        Position normalized = normalize(position);
        return cells[normalized.y][normalized.x];
    }

    std::shared_ptr<MapCell> at(const Entity &entity)
    {
        return at(entity.position);
    }

    std::shared_ptr<MapCell> at(const Entity *entity)
    {
        return at(entity->position);
    }

    std::shared_ptr<MapCell> at(const std::shared_ptr<Entity> &entity)
    {
        return at(entity->position);
    }

    int calculate_distance(const Position &source, const Position &target)
    {
        const auto &normalized_source = normalize(source);
        const auto &normalized_target = normalize(target);

        const int dx = std::abs(normalized_source.x - normalized_target.x);
        const int dy = std::abs(normalized_source.y - normalized_target.y);

        const int toroidal_dx = std::min(dx, width - dx);
        const int toroidal_dy = std::min(dy, height - dy);

        return toroidal_dx + toroidal_dy;
    }

    void normalize(Position& position)
    {
        position.x = ((position.x % width) + width) % width;
        position.y = ((position.y % height) + height) % height;
    }

    Position normalize(const Position& position)
    {
        const int x = ((position.x % width) + width) % width;
        const int y = ((position.y % height) + height) % height;
        return {x, y};
    }

    std::vector<Direction> get_unsafe_moves(const Position &source, const Position &destination)
    {
        const auto &normalized_source = normalize(source);
        const auto &normalized_destination = normalize(destination);

        const int dx = std::abs(normalized_source.x - normalized_destination.x);
        const int dy = std::abs(normalized_source.y - normalized_destination.y);
        const int wrapped_dx = width - dx;
        const int wrapped_dy = height - dy;

        std::vector<Direction> possible_moves;

        double eastCost = at(normalized_source.directional_offset(Direction::EAST))->halite;
        double westCost = at(normalized_source.directional_offset(Direction::WEST))->halite;
        double northCost = at(normalized_source.directional_offset(Direction::NORTH))->halite;
        double southCost = at(normalized_source.directional_offset(Direction::SOUTH))->halite;

        Direction dirX = Direction::STILL;
        if (normalized_source.x < normalized_destination.x)
        {
            dirX = dx > wrapped_dx ? Direction::WEST : Direction::EAST;
        }
        else if (normalized_source.x > normalized_destination.x)
        {
            dirX = dx < wrapped_dx ? Direction::WEST : Direction::EAST;
        }

        Direction dirY = Direction::STILL;
        if (normalized_source.y < normalized_destination.y)
        {
            dirY = dy > wrapped_dy ? Direction::NORTH : Direction::SOUTH;
        }
        else if (normalized_source.y > normalized_destination.y)
        {
            dirY = dy < wrapped_dy ? Direction::NORTH : Direction::SOUTH;
        }

        if (dirX == Direction::EAST && dirY == Direction::NORTH)
        {
            if (eastCost <= northCost)
            {
                possible_moves.push_back(Direction::EAST);
                possible_moves.push_back(Direction::NORTH);
            }
            else
            {
                possible_moves.push_back(Direction::NORTH);
                possible_moves.push_back(Direction::EAST);
            }
        }
        else if (dirX == Direction::EAST && dirY == Direction::SOUTH)
        {
            if (eastCost <= southCost)
            {
                possible_moves.push_back(Direction::EAST);
                possible_moves.push_back(Direction::SOUTH);
            }
            else
            {
                possible_moves.push_back(Direction::SOUTH);
                possible_moves.push_back(Direction::EAST);
            }
        }

        else if (dirX == Direction::WEST && dirY == Direction::NORTH)
        {
            if (westCost <= northCost)
            {
                possible_moves.push_back(Direction::WEST);
                possible_moves.push_back(Direction::NORTH);
            }
            else
            {
                possible_moves.push_back(Direction::NORTH);
                possible_moves.push_back(Direction::WEST);
            }
        }
        else if (dirX == Direction::WEST && dirY == Direction::SOUTH)
        {
            if (westCost <= southCost)
            {
                possible_moves.push_back(Direction::WEST);
                possible_moves.push_back(Direction::SOUTH);
            }
            else
            {
                possible_moves.push_back(Direction::SOUTH);
                possible_moves.push_back(Direction::WEST);
            }
        }
        else if (dirX == Direction::WEST && dirY == Direction::STILL)
        {
            possible_moves.push_back(Direction::WEST);
            possible_moves.push_back(Direction::NORTH);
            possible_moves.push_back(Direction::SOUTH);
        }
        else if (dirX == Direction::EAST && dirY == Direction::STILL)
        {
            possible_moves.push_back(Direction::EAST);
            possible_moves.push_back(Direction::SOUTH);
            possible_moves.push_back(Direction::NORTH);
        }
        else if (dirX == Direction::STILL && dirY == Direction::NORTH)
        {
            possible_moves.push_back(Direction::NORTH);
            possible_moves.push_back(Direction::EAST);
            possible_moves.push_back(Direction::WEST);
        }
        else if (dirX == Direction::STILL && dirY == Direction::SOUTH)
        {
            possible_moves.push_back(Direction::SOUTH);
            possible_moves.push_back(Direction::WEST);
            possible_moves.push_back(Direction::EAST);
        }

        // if (normalized_source.x == normalized_destination.x) {
        //     if(normalized_source.y < normalized_destination.y)
        //         possible_moves.push_back(Direction::WEST);
        //     else if(normalized_source.y > normalized_destination.y)
        //         possible_moves.push_back(Direction::EAST);
        // }

        // if (normalized_source.y == normalized_destination.y) {
        //     if(normalized_source.x < normalized_destination.x)
        //         possible_moves.push_back(Direction::NORTH);
        //     else  if(normalized_source.x > normalized_destination.x)
        //         possible_moves.push_back(Direction::SOUTH);
        // }

        return possible_moves;
    }

    Direction naive_navigate(std::shared_ptr<Ship> &ship, const Position& destination)
    {
        // get_unsafe_moves normalizes for us
        if (ship->position == destination)
        {
            return Direction::STILL;
        }

        for (const auto &direction : get_unsafe_moves(ship->position, destination))
        {
            const Position &target_pos = ship->position.directional_offset(direction);
            if (not at(target_pos)->is_occupied())
            {
                at(target_pos)->mark_unsafe(ship);
                return direction;
            }
        }

        return Direction::STILL;
    }

    void navigate(const std::vector<std::shared_ptr<Ship>>& ships, const Position& destination)
    {
        std::vector<std::shared_ptr<Ship>> shipsToPlan(ships.size());
        for(auto const& ship : ships)
        {
            if(ship->destination == destination)
                return;
            shipsToPlan.emplace_back(ship);
        }
        log::log("Path planning for " + std::to_string(shipsToPlan.size()) + " ships.");
        
        int maxSteps = 20;
        // For now no multi-robot planning, simply plan one by one
        for(const auto& ship : shipsToPlan)
        {
            const std::shared_ptr<MapCell>& start = at(ship->position);
            const std::shared_ptr<MapCell>& cur = start;
            const std::shared_ptr<MapCell>& dest = at(ship->destination);
            std::vector<const std::shared_ptr<MapCell>> openPositions(width*height);
            openPositions.push_back(start);
            while(not openPositions.empty() || start == dest)
            {
                std::sort(openPositions.begin(), openPositions.end());
                const auto& openPos = openPositions.front();
                
                for(const auto& dir : ALL_DIRECTIONS)
                {
                    openPos->directional_offset(dir);
                }

                openPositions.erase(openPositions.);
            }
        }
       
    }   

    void _update();
    static std::unique_ptr<GameMap> _generate();
};
} // namespace hlt
