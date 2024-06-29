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

std::shared_ptr<Player> PlayerTokens::FindPlayerByToken(const Token &token) {
    auto it = token_to_player_.find(token);
    if (it != token_to_player_.end()) {
        return it->second;
    }
    return nullptr;
}

void PlayerTokens::PrintToken() {
    for (const auto& token : token_to_player_) {
        std::cout << *token.first << std::endl;
    }
}


}   //namepsace app
