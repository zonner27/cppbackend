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

    Application(model::Game& game, net::io_context& ioc, uint32_t tick_period, bool randomize_spawn_points) :
        game_{game}, ioc_{ioc}, tick_period_{tick_period}, randomize_spawn_points_{randomize_spawn_points}, api_strand_{std::make_shared<Strand>(net::make_strand(ioc))} {}

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
    uint32_t GetTickPeriod();
    bool GetRandomizeSpawnPoints();

private:
    model::Game game_;
    uint32_t tick_period_ = 0;
    bool randomize_spawn_points_ = false;
    net::io_context& ioc_;
    std::vector<std::shared_ptr<Player>> players_;
    PlayerTokens player_tokens_;
    std::shared_ptr<Strand> api_strand_;

};


} //namespace app
