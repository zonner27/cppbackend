#pragma once

#include <random>
#include <string>
#include <iomanip>
#include <unordered_map>
#include "tagged.h"
#include "model.h"

namespace detail {
struct TokenTag {
};
}  // namespace detail

namespace app {


class Player {
public:
    Player(int id) : playerId(nextId++) {}
    uint32_t GetPlayerId() const { return playerId; }
private:
    static uint32_t nextId;
    uint32_t playerId  = 0;
    model::GameSession* session_;
    model::Dog* dog_;
};

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    Token generateToken();
    Token AddPlayer(Player& player);
    Player* FindPlayerByToken(const Token& token);

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

    //using MapTokenHasher = util::TaggedHasher<detail::TokenTag>;
    //using MapTokenToPlayer = std::unordered_map<Token, Player*, MapTokenHasher>;

    using MapTokenHasher = util::TaggedHasher<Token>;
    using MapTokenToPlayer = std::unordered_map<Token, Player*, MapTokenHasher>;
    MapTokenToPlayer token_to_player_;

};
//using MapIdHasher = util::TaggedHasher<Map::Id>;
//using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

//std::vector<Map> maps_;
//MapIdToIndex map_id_to_index_


} // namespace app
