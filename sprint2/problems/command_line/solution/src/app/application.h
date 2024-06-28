#pragma once

#include <vector>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <functional>

#include "players.h"
#include "player_tokens.h"
#include "../model/game.h"
#include "../time/ticker.h"

namespace app {

namespace net = boost::asio;

class Application {

public:
    using Strand = net::strand<net::io_context::executor_type>;

    Application(model::Game& game, net::io_context& ioc, uint32_t tick_period, bool randomize_spawn_points) :
        game_{game},
        ioc_{ioc},
        tick_period_{tick_period},
        randomize_spawn_points_{randomize_spawn_points},
        api_strand_{std::make_shared<Strand>(net::make_strand(ioc))} {

        if(tick_period_.count() != 0){
            ticker_ = std::make_shared<time_tiker::Ticker>(
                *api_strand_,
                tick_period_,
                [this](std::chrono::milliseconds delta) { UpdateGameState(delta); }
            );
            ticker_->Start();
        }
    }

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    std::pair<Token, Player::ID> JoinGame(std::string userName, const model::Map* map);
    Player* FindByDogNameAndMapId(const std::string& dogName, const std::string& mapId);
    std::vector<std::shared_ptr<Player>>& GetPlayers();
    PlayerTokens& GetPlayerTokens();
    model::Game& GetGame();
    std::shared_ptr<Strand> GetStrand();
    std::chrono::milliseconds GetTickPeriod();
    bool GetRandomizeSpawnPoints();

    void UpdateGameState(const std::chrono::milliseconds& time_delta) {
        for (auto session : game_.GetAllSession()) {
            session->SetDogsCoordinatsByTime(time_delta.count());
        }
    }

private:
    model::Game game_;
    std::chrono::milliseconds tick_period_;
    bool randomize_spawn_points_ = false;
    net::io_context& ioc_;
    std::vector<std::shared_ptr<Player>> players_;
    PlayerTokens player_tokens_;
    std::shared_ptr<Strand> api_strand_;

    std::shared_ptr<time_tiker::Ticker> ticker_;

};


} //namespace app
