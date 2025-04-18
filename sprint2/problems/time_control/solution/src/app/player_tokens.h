#pragma once

#include "players.h"

namespace detail {
struct TokenTag {
};
}  // namespace detail

namespace app {

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    PlayerTokens() = default;
    Token generateToken();
    Token AddPlayer(std::shared_ptr<Player> player);    //std::shared_ptr<Player> player   Player&
    std::shared_ptr<Player> FindPlayerByToken(const Token& token);  //std::shared_ptr<Player> player   Player*
    void PrintToken();

private:

    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    // Чтобы сгенерировать токен, получите из generator1_ и generator2_
    // два 64-разрядных числа и, переведя их в hex-строки, склейте в одну.
    // Вы можете поэкспериментировать с алгоритмом генерирования токенов,
    // чтобы сделать их подбор ещё более затруднительным

    using MapTokenHasher = util::TaggedHasher<Token>;
    using MapTokenToPlayer = std::unordered_map<Token, std::shared_ptr<Player>, MapTokenHasher>;
    MapTokenToPlayer token_to_player_;

};


}   //namepsace app
