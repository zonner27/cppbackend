#pragma once

#include <vector>
#include <string>

#include "model.h"
#include "game_session.h"

namespace model {

using namespace std::literals;

class Game {
public:
    using Maps = std::vector<Map>;

    void AddMap(Map map);
    void AddSession(std::shared_ptr<GameSession> session);

    std::vector<std::shared_ptr<GameSession>>& GetAllSession();
    std::shared_ptr<GameSession> FindValidSession(const Map* map);

    const Maps& GetMaps() const noexcept;
    const Map* FindMap(const Map::Id& id) const noexcept;

    void SetDefaultDogSpeed(double defaultDogSpeed);
    const double GetDefaultDogSpeed();

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;

    std::vector<std::shared_ptr<GameSession>> sessions_;
    double defaultDogSpeed_ = 1;
};


} // namespace model
