#include <random>
#include <ctime>
#include <string>
#include <vector>
#include <algorithm>

#include "hlt/game.hpp"
#include "hlt/constants.hpp"
#include "hlt/log.hpp"

using namespace hlt;
using std::to_string;

class Commander
{
  public:
    Commander(unsigned int seed, std::string name)
        : mRng(seed), mName(name), mSelf(mGame.me), mMap(mGame.game_map)
    {
        log::log("Initializing bot! My Player ID is " + to_string(mGame.my_id) + ". Bot rng seed is " + to_string(seed) + ".");

        mGame.ready("okkhen0"); // 2 second timer starts
        log::log("Successfully readied bot! My Player ID is " + to_string(mGame.my_id) + ".");
    }

    bool turn()
    {
        mGame.update_frame();
        std::vector<Command> commands;
        std::vector<Position> claimedPositions;

        // Determine to mine positions
        double factor = 2.0;
        std::vector<Position> minePositions;
        std::vector<std::vector<MapCell>> cells = mMap->cells;
        for (const auto &col_iterator : cells)
        {
            const auto &col = col_iterator;
            for (const auto &row_iterator : col)
            {
                const auto &cell = row_iterator;  
                if (cell.halite > mMap->averageHallite * factor)
                {
                    minePositions.emplace_back(cell.position);
                }
            }
        }

        // Sort on halite amount
        double distanceWeight = 1.0;
        std::sort(minePositions.begin(), minePositions.end(),[&](const Position& a, const Position& b) {
            double distanceA = mMap->calculate_distance(a, mSelf->shipyard->position) * distanceWeight;
            double distanceB = mMap->calculate_distance(b, mSelf->shipyard->position) * distanceWeight;
            return mMap->at(a)->halite * 1.0/distanceA > mMap->at(b)->halite * 1.0/distanceB;   
        });

        for(const auto& pos : minePositions)
        {
            const auto cell = mMap->at(pos);
            log::log("Marked " + cell->position.to_string() + " as a mining position, distance to yard: " + 
                std::to_string(mMap->calculate_distance(cell->position, mSelf->shipyard->position)) + 
                " (" + std::to_string(cell->halite) + "/" + std::to_string(mMap->averageHallite) + "/" + std::to_string(mMap->averageHallite * factor) + ").");
        }

        int currentMinePostion = 0;
        std::vector<std::pair<EntityId, std::shared_ptr<Ship>>> sortedShips(mSelf->ships.begin(), mSelf->ships.end());
        std::sort(sortedShips.begin(), sortedShips.end(), [&](const std::pair<EntityId, std::shared_ptr<Ship>>& a, const std::pair<EntityId, std::shared_ptr<Ship>>& b) {
            return a.first < b.first;   
        });

        for (const auto &ship_iterator : sortedShips)
        {
            std::shared_ptr<Ship> ship = ship_iterator.second;

            Position minePos;
            if (ship->halite >= mMap->at(ship)->halite * 0.80 && mMap->at(ship)->halite < 50)
            {
                minePos = minePositions.at(currentMinePostion % (minePositions.size() - 1));
                currentMinePostion++;
            }
            else
            {
                minePos = ship->position;
            }
            
            if (ship->halite > constants::MAX_HALITE * 0.8)
            {
                auto dir = mMap->naive_navigate(ship, mSelf->shipyard->position);
                log::log(to_string(ship->id) + " going to spawn at " + mSelf->shipyard->position.to_string());
                commands.push_back(ship->move(dir));
            }
            else if (ship->position != minePos)
            {
                Position randomPos(mMap->width, mMap->height);
                auto dir = mMap->naive_navigate(ship, minePos);
                log::log(to_string(ship->id) + " going to mine at " + minePos.to_string());
                commands.push_back(ship->move(dir));
            }
            else
            {
                auto dir = mMap->naive_navigate(ship, ship->position);
                log::log(to_string(ship->id) + " mining at " + mSelf->shipyard->position.to_string());
                commands.push_back(ship->move(dir));
            }
        }

        if (mGame.turn_number <= constants::MAX_TURNS * 0.66 && mSelf->ships.size() < 25 && mSelf->halite >= constants::SHIP_COST && not mMap->at(mSelf->shipyard->position)->is_occupied())
        {
            commands.push_back(mSelf->shipyard->spawn());
            claimedPositions.push_back(mSelf->shipyard->position);
        }
        return mGame.end_turn(commands);
    }

  private:
    Game mGame;
    std::mt19937 mRng;
    std::string mName;
    std::shared_ptr<Player> mSelf;
    std::unique_ptr<GameMap> &mMap;
};
