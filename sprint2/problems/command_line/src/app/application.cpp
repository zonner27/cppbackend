#include "application.h"

namespace app {

std::pair<app::Token, app::Player::ID> app::Application::JoinGame(std::string userName, const model::Map *map) {

    //std::cout << "randlomie = " << randomize_spawn_points_ << std::endl;
    std::shared_ptr<model::Dog> dog = std::make_shared<model::Dog>(userName);
    std::shared_ptr<model::GameSession> validSession = game_.FindValidSession(map);
    validSession->AddDog(dog, randomize_spawn_points_);
    std::shared_ptr<Player> player = std::make_shared<Player>(dog, validSession);
    players_.push_back(player);
    Token authToken = player_tokens_.AddPlayer(player);
    Player::ID playerId = player->GetPlayerId();

    return make_pair(authToken, playerId);
}

Player *Application::FindByDogNameAndMapId(const std::string &dogName, const std::string &mapId) {
    for (const auto& player : players_) {
        if (player->GetDog()->GetName() == dogName && *player->GetSession()->GetId() == mapId) {
            return player.get();
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Player> > &Application::GetPlayers() {
    return players_;
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

unsigned int Application::GetTickPeriod() {
    return tick_period_;
}

bool Application::GetRandomizeSpawnPoints() {
    return randomize_spawn_points_;
}


} //namespace app
