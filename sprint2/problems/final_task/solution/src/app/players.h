#pragma once

#include <random>
#include <string>
#include <iomanip>
#include <unordered_map>
#include "../tagged.h"
#include "../model/model.h"
#include "../model/game_session.h"

namespace app {

class Player {
public:
    using ID = uint32_t;

    Player() = default;
    Player(std::shared_ptr<model::Dog> dog, std::shared_ptr<model::GameSession> game_session) : playerId(dog->GetId()), dog_(dog), session_(game_session) {}

    ID GetPlayerId() const;
    std::shared_ptr<model::Dog> GetDog() const;
    std::shared_ptr<model::GameSession> GetSession() const;

private:
    ID playerId  = 0;
    std::shared_ptr<model::GameSession> session_;
    std::shared_ptr<model::Dog>  dog_;
};

} // namespace app
