#include "player_tokens.h"

namespace app {

Token app::PlayerTokens::GenerateToken() {
    auto random_number1 = generator1_();
    auto random_number2 = generator2_();

    std::stringstream sstr;
    sstr << std::hex << std::setw(16) << std::setfill('0') << random_number1
         << std::hex << std::setw(16) << std::setfill('0') << random_number2;

    return Token(sstr.str());
}

Token PlayerTokens::AddPlayer(std::shared_ptr<Player> player) {
    Token token = GenerateToken();
    token_to_player_[token] = player;
    return token;
}

void PlayerTokens::AddPlayerToken(std::shared_ptr<Player> player, const Token &token) {
    token_to_player_[token] = player;
}

std::shared_ptr<Player> PlayerTokens::FindPlayerByToken(const Token &token) {
    auto it = token_to_player_.find(token);
    if (it != token_to_player_.end()) {
        return it->second;
    }
    return nullptr;
}

PlayerTokens::MapTokenToPlayer PlayerTokens::GetPlayerToken() {
    return token_to_player_;
}

void PlayerTokens::RemovePlayerById(uint32_t player_id) {
    auto it = std::find_if(token_to_player_.begin(), token_to_player_.end(),
                           [player_id](const std::pair<Token, std::shared_ptr<Player>>& pair) {
        return pair.second->GetPlayerId() == player_id;
    });

    if (it != token_to_player_.end()) {
        token_to_player_.erase(it);
    }
}


}   //namepsace app
