#include "players.h"

namespace app {

Token app::PlayerTokens::generateToken() {
    auto random_number1 = generator1_();
    auto random_number2 = generator2_();

    std::stringstream sstr;
    sstr << std::hex << std::setw(16) << std::setfill('0') << random_number1
         << std::hex << std::setw(16) << std::setfill('0') << random_number2;

    return Token(sstr.str());
}

Token PlayerTokens::AddPlayer(Player &player) {
    Token token = generateToken();
    token_to_player_[token] = &player;
    return token;
}

Player *PlayerTokens::FindPlayerByToken(const Token &token) {
    auto it = token_to_player_.find(token);
    if (it != token_to_player_.end()) {
        return it->second;
    }
    return nullptr;
}

//uint32_t Player::nextId = 0;

} // namespace app
