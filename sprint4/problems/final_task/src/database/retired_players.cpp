#include "retired_players.h"

namespace domain {

const RetiredPlayersId &RetiredPlayers::GetId() const noexcept {
    return id_;
}

const std::string &RetiredPlayers::GetName() const noexcept {
    return name_;
}

uint32_t RetiredPlayers::GetPlayerId() const noexcept {
    return id_player_;
}

uint32_t RetiredPlayers::GetScore() const noexcept {
    return score_;
}

uint32_t RetiredPlayers::GetPlayTime() const noexcept {
    return play_time_;
}

}  // namespace domain
