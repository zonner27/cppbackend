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

    Application(model::Game& game, net::io_context& ioc) : game_{game}, ioc_{ioc}, api_strand_{std::make_shared<Strand>(net::make_strand(ioc))} {}

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

private:
    model::Game game_;
    net::io_context& ioc_;
    std::vector<std::shared_ptr<Player>> players_;
    PlayerTokens player_tokens_;
    std::shared_ptr<Strand> api_strand_;

};


} //namespace app
