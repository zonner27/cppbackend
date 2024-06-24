#pragma once

#include <vector>
#include "players.h"
#include "player_tokens.h"


namespace app {

class Application {


private:
    std::vector<std::shared_ptr<Player>> players_;
    PlayerTokens player_tokens_;
};



} //namespace app
