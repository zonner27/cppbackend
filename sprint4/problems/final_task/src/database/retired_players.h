#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#include "../tagged_uuid.h"

namespace domain {

namespace detail {
struct RetiredPlayersTag {};
}  // namespace detail

using RetiredPlayersId = util::TaggedUUID<detail::RetiredPlayersTag>;

class RetiredPlayers {
public:
    RetiredPlayers(RetiredPlayersId id, std::string name, uint32_t id_player,
                   uint32_t score, uint32_t play_time_)
        : id_(std::move(id))
        , name_(std::move(name))
        , id_player_(id_player)
        , score_(score)
        , play_time_(play_time_){
    }

    const RetiredPlayersId& GetId() const noexcept;
    const std::string& GetName() const noexcept;
    uint32_t GetPlayerId() const noexcept;
    uint32_t GetScore() const noexcept;
    uint32_t GetPlayTime() const noexcept;

private:
    RetiredPlayersId id_;
    std::string name_{};
    uint32_t id_player_{0};
    uint32_t score_{0};
    uint32_t play_time_{0};
};

class RetiredPlayersRepository {
public:
    virtual void SaveRetiredPlayers(const std::vector<RetiredPlayers>& retired_players) = 0;
    virtual std::vector<RetiredPlayers> GetTableRecord(size_t start, size_t maxItems) = 0;

protected:
    ~RetiredPlayersRepository() = default;
};

}  // namespace domain
