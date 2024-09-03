#include "players.h"

namespace app {

Player::ID Player::GetPlayerId() const {
    return playerId;
}

std::weak_ptr<model::Dog> Player::GetDog() const {
    return dog_;
}

std::weak_ptr<model::GameSession> Player::GetSession() const {
    return session_;
}


} // namespace app
