#pragma once

#include "types.hpp"
#include "map_cell.hpp"

#include <vector>

namespace hlt
{
struct GameMap
{
    int width;
    int height;
    int nrCells;
    Halite averageHallite;
    Halite totalHallite;
    std::vector<std::vector<MapCell>> cells;

    MapCell *at(const Position &position)
    {
        Position normalized = normalize(position);
        return &cells[normalized.y][normalized.x];
    }

    MapCell *at(const Entity &entity)
    {
        return at(entity.position);
    }

    MapCell *at(const Entity *entity)
    {
        return at(entity->position);
    }

    MapCell *at(const std::shared_ptr<Entity> &entity)
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

    Position normalize(const Position &position)
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

    Direction naive_navigate(std::shared_ptr<Ship> &ship, const Position &destination)
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

    void _update();
    static std::unique_ptr<GameMap> _generate();
};
} // namespace hlt
