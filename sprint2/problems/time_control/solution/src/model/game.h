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

    void AddSession(std::shared_ptr<GameSession> session) {
        sessions_.push_back(session);
    }

    std::vector<std::shared_ptr<GameSession>>& GetAllSession() {
        return sessions_;
    }

    std::shared_ptr<GameSession> FindValidSession(const Map* map) {
        for (const auto& session : sessions_) {
            if (session->GetMapName() == map->GetName() && session->GetDogsCount() < constants::MAXPLAYERSINMAP)
                return session;
        }
        auto newSession = std::make_shared<GameSession>(map);
        AddSession(newSession);
        return newSession;
    }

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    void SetDefaultDogSpeed(double defaultDogSpeed){
        defaultDogSpeed_= defaultDogSpeed;
    }

    const double GetDefaultDogSpeed() {
        return defaultDogSpeed_;
    }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;

    std::vector<std::shared_ptr<GameSession>> sessions_;
    double defaultDogSpeed_ = 1;
};


} // namespace model
