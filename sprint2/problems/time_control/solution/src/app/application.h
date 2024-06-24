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

    Application(model::Game& game, net::io_context& ioc) :
        game_{game},
        ioc_{ioc}
        {}


private:
    model::Game game_;
    net::io_context& ioc_;
    std::vector<std::shared_ptr<Player>> players_;
    PlayerTokens player_tokens_;

};


} //namespace app
