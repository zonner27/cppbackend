#include "player_serialization.h"

namespace serialization {

app::Player::ID PlayersRepr::RestorePlayerID() const {
    return id_;
}

app::Token PlayersRepr::RestoreToken() const {
    return app::Token(token_);
}


} //serialization
