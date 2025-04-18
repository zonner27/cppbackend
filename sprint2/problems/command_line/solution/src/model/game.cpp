#include "game.h"


namespace model {

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

void Game::AddSession(std::shared_ptr<GameSession> session) {
    sessions_.push_back(session);
}

std::vector<std::shared_ptr<GameSession> > &Game::GetAllSession() {
    return sessions_;
}

std::shared_ptr<GameSession> Game::FindValidSession(const Map *map) {
    for (const auto& session : sessions_) {
        if (session->GetMapName() == map->GetName() && session->GetDogsCount() < constants::MAXPLAYERSINMAP)
            return session;
    }
    auto newSession = std::make_shared<GameSession>(map);
    AddSession(newSession);
    return newSession;
}

const Game::Maps &Game::GetMaps() const noexcept {
    return maps_;
}

const Map *Game::FindMap(const Map::Id &id) const noexcept {
    if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
        return &maps_.at(it->second);
    }
    return nullptr;
}

void Game::SetDefaultDogSpeed(double defaultDogSpeed){
    defaultDogSpeed_= defaultDogSpeed;
}

const double Game::GetDefaultDogSpeed() {
    return defaultDogSpeed_;
}

} // namespace model
