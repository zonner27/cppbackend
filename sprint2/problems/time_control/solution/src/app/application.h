#pragma once

#include <vector>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include "players.h"
#include "player_tokens.h"
#include "../model/game.h"


namespace app {

namespace net = boost::asio;

class Application {

public:
    using Strand = net::strand<net::io_context::executor_type>;

    Application(model::Game& game, net::io_context& ioc) : game_{game}, ioc_{ioc}, api_strand_{std::make_shared<Strand>(net::make_strand(ioc))} {   //api_strand_{net::make_strand(ioc)}

    }

    std::pair<Token, Player::ID> JoinGame(std::string userName, const model::Map* map) {

        std::shared_ptr<model::Dog> dog = std::make_shared<model::Dog>(userName);
        std::shared_ptr<model::GameSession> validSession = game_.FindValidSession(map);
        validSession->AddDog(dog);
        std::shared_ptr<Player> player = std::make_shared<Player>(dog, validSession);
        players_.push_back(player);
        Token authToken = player_tokens_.AddPlayer(player);
        Player::ID playerId = player->GetPlayerId();

        return make_pair(authToken, playerId);
    }

    Player* FindByDogNameAndMapId(const std::string& dogName, const std::string& mapId) {
        for (const auto& player : players_) {
            if (player->GetDog()->GetName() == dogName && *player->GetSession()->GetId() == mapId) {
                return player.get();
            }
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Player>>& GetPlayers() {
        return players_;
    }

    PlayerTokens& GetPlayerTokens(){
        return player_tokens_;
    }

    model::Game& GetGame() {
        return game_;
    }

    std::shared_ptr<Strand> GetStrand() {
        return api_strand_;
    }

private:
    model::Game game_;
    net::io_context& ioc_;
    std::vector<std::shared_ptr<Player>> players_;
    PlayerTokens player_tokens_;
    std::shared_ptr<Strand> api_strand_;

};


} //namespace app
