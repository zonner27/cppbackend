#pragma once

#include "model_serialization.h"
#include "../app/players.h"
#include "../app/player_tokens.h"

namespace serialization {

class PlayersRepr {
public:
    PlayersRepr() = default;
    PlayersRepr(app::Player::ID player_id, const app::Token& token):
        id_(player_id),
        token_(*token) {};
    PlayersRepr(PlayersRepr&& other) = default;

    [[nodiscard]] app::Player::ID RestorePlayerID() const;
    [[nodiscard]] app::Token  RestoreToken() const;

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& token_;
    }
private:
    app::Player::ID id_;
    std::string token_;
};

} //serialization
