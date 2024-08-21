#include "application.h"

namespace app {

std::pair<app::Token, app::Player::ID> app::Application::JoinGame(std::string userName, const model::Map *map) {

    std::shared_ptr<model::Dog> dog = std::make_shared<model::Dog>(userName);
    std::shared_ptr<model::GameSession> validSession = game_.FindValidSession(map, tick_period_, ioc_);
    validSession->AddDog(dog, randomize_spawn_points_);
    std::shared_ptr<Player> player = std::make_shared<Player>(dog, validSession);
    players_.push_back(player);
    Token authToken = player_tokens_.AddPlayer(player);
    Player::ID playerId = player->GetPlayerId();

    return make_pair(authToken, playerId);
}

Player* Application::FindByDogNameAndMapId(const std::string &dogName, const std::string &mapId) {

    auto it = std::find_if(players_.begin(), players_.end(),
           [&dogName, &mapId](const std::shared_ptr<Player>& player) {
               auto dog = player->GetDog().lock();
               auto session = player->GetSession().lock();
               return dog && session && dog->GetName() == dogName && *session->GetId() == mapId;
           });

    return it != players_.end() ? it->get() : nullptr;

//    for (const auto& player : players_) {
//        if (player->GetDog().lock()->GetName() == dogName && *player->GetSession().lock()->GetId() == mapId) {
//            return player.get();
//        }
//    }
//    return nullptr;
}


PlayerTokens &Application::GetPlayerTokens(){
    return player_tokens_;
}

model::Game &Application::GetGame() {
    return game_;
}

std::shared_ptr<Application::Strand> Application::GetStrand() {
    return api_strand_;
}

std::chrono::milliseconds Application::GetTickPeriod() {
    return tick_period_;
}

bool Application::GetRandomizeSpawnPoints() {
    return randomize_spawn_points_;
}

void Application::UpdateGameState(const std::chrono::milliseconds &time_delta) {
    for (auto session : game_.GetAllSession()) {
        session->UpdateSessionByTime(time_delta);
    }
}


} //namespace app
