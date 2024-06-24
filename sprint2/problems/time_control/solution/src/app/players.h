#pragma once

#include <random>
#include <string>
#include <iomanip>
#include <unordered_map>
#include "../tagged.h"
#include "../model/model.h"
#include "../model/game_session.h"
#include <iostream> //del


namespace app {


class Player {
public:
    Player() = default;
    Player(std::shared_ptr<model::Dog> dog, std::shared_ptr<model::GameSession> game_session) : playerId(dog->GetId()), dog_(dog), session_(game_session) {
    }

    uint32_t GetPlayerId() const {
        return playerId;
    }

    std::shared_ptr<model::Dog> GetDog() const {
        return dog_;
    }

    std::shared_ptr<model::GameSession> GetSession() const {
        return session_;
    }


private:
    uint32_t playerId  = 0;
    std::shared_ptr<model::GameSession> session_;       //std::shared_ptr<model::GameSession> session
    std::shared_ptr<model::Dog>  dog_;                  //std::shared_ptr<model::Dog>  dog_
};

class Players {
public:
    Player& Add(std::shared_ptr<model::Dog> dog, std::shared_ptr<model::GameSession> session) {
        auto it = players_.find(dog);
        if (it != players_.end()) {
            return it->second;
        }

        Player player(dog, session);
        players_[dog] = player;
        return players_[dog];
    }

    Player* FindByDogNameAndMapId(const std::string& dogName, const std::string& mapId) {
        for (auto& [dog, player] : players_) {
            //std::cout << "___dogname " << dog->GetName() << " - " << dogName << std::endl;
            //std::cout << "___map " << *player.GetSession()->GetId() << " - " << mapId << std::endl;
            if (dog->GetName() == dogName && *player.GetSession()->GetId() == mapId) {

                return &player;
            }
        }
        return nullptr;
    }

    std::unordered_map<std::shared_ptr<model::Dog> , Player> GetPlayers() {
        return players_;
    }

private:
    std::unordered_map<std::shared_ptr<model::Dog> , Player> players_;
};





} // namespace app
