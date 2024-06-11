#pragma once

#include <random>
#include <string>
#include <iomanip>
#include <unordered_map>
#include "tagged.h"
#include "model.h"
#include <iostream> //del

namespace detail {
struct TokenTag {
};
}  // namespace detail

namespace app {


class Player {
public:
    Player() = default;
    Player(model::Dog* dog, model::GameSession* game_session) : playerId(dog->GetId()), dog_(dog), session_(game_session) {}    //nextId++
    uint32_t GetPlayerId() const {
        return playerId;
    }
    model::Dog* GetDog() const {
        return dog_;
    }
    model::GameSession* GetSession() const {
        return session_;
    }


private:
    //static uint32_t nextId;
    uint32_t playerId  = 0;
    model::GameSession* session_;
    model::Dog* dog_;
};

class Players {
public:
    Player& Add(model::Dog* dog, model::GameSession* session) {
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

    std::unordered_map<model::Dog*, Player> GetPlayers() {
        return players_;
    }


private:
    std::unordered_map<model::Dog*, Player> players_;
};

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    Token generateToken();
    Token AddPlayer(Player& player);
    Player* FindPlayerByToken(const Token& token);
    void PrintToken() {
        for (const auto& token : token_to_player_) {
            std::cout << *token.first << std::endl;
        }
    }

private:

    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    // Чтобы сгенерировать токен, получите из generator1_ и generator2_
    // два 64-разрядных числа и, переведя их в hex-строки, склейте в одну.
    // Вы можете поэкспериментировать с алгоритмом генерирования токенов,
    // чтобы сделать их подбор ещё более затруднительным

    using MapTokenHasher = util::TaggedHasher<Token>;
    using MapTokenToPlayer = std::unordered_map<Token, Player*, MapTokenHasher>;
    MapTokenToPlayer token_to_player_;

};

class Application {

};


} // namespace app
