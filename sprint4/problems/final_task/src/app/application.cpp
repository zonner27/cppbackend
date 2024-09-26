#include "application.h"

namespace app {

std::pair<app::Token, app::Player::ID> app::Application::JoinGame(std::string userName, const model::Map *map) {

    std::shared_ptr<model::Dog> dog = std::make_shared<model::Dog>(userName);
    std::shared_ptr<model::GameSession> validSession = game_.FindValidSession(map, tick_period_, ioc_);
    if (validSession->GetDogs().size() == 0) {
        ConnectGameSessionSignals(validSession);
    }
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

void Application::LoadGame(std::filesystem::__cxx11::path game_save_path, std::chrono::milliseconds save_period) {
    game_save_path_ = game_save_path;
    save_period_ = save_period;

    if (fs::exists(game_save_path_.value())) {
        LoadGameFromArchive();
    } else {
        return;
    }

    if(save_period.count() == 0 || tick_period_.count() == 0) {
        return;
    }

    save_ticker_ = std::make_shared<time_tiker::Ticker>(
            *api_strand_,
            save_period,
            [self_weak = weak_from_this()](const std::chrono::milliseconds& delta) {

        if(auto self = self_weak.lock()) {
            self->SaveGame();
        }
    });

    save_ticker_->Start();
}

void Application::LoadGameFromArchive() {

    std::fstream infstream;
    infstream.open(game_save_path_.value(), std::ios_base::in);
    if(!infstream.is_open()) {
        return;
    }

    std::vector<serialization::GameSessionResp> sessions_resp;
    std::vector<serialization::PlayersRepr> players_resp;

    boost::archive::text_iarchive iarchive{infstream};
    iarchive >> sessions_resp >> players_resp;
    infstream.close();

    for (auto& session_resp : sessions_resp) {
        auto new_session = std::make_shared<model::GameSession>(
                    game_.FindMap(session_resp.RestoreMapId()),
                    tick_period_,
                    game_.GetLootGeneratorConfig(),
                    ioc_);
        for(auto& lost_obj_resp : session_resp.GetLostObjectsResp()) {
            auto lost_object = std::make_shared<model::LostObject>(std::move(lost_obj_resp.Restore()));
            new_session->AddLostObject(lost_object);
        }

        for(auto& dog_resp : session_resp.GetDogsResp()) {
            auto dog = std::make_shared<model::Dog>(std::move(dog_resp.Restore()));
            new_session->AddDog(dog);
            std::shared_ptr<Player> player = std::make_shared<Player>(dog, new_session);
            players_.push_back(player);

            for (auto& player_resp : players_resp) {
                if (dog->GetId() != player_resp.RestorePlayerID()) {
                    continue;
                }
                player_tokens_.AddPlayerToken(player, player_resp.RestoreToken());
            }
        }

        game_.AddSession(new_session);
        new_session->Run();
    }
}

void Application::SaveGameByTime(const std::chrono::milliseconds &delta_time) {
    static int period = save_period_.count();
    period -= delta_time.count();
    if(period <= 0) {
        SaveGame();
        period = save_period_.count();
    }
}

void Application::SaveGame() {
    if (!game_save_path_) {
        return;
    }

    std::vector<serialization::GameSessionResp> sessions_resp;
    for(auto session : game_.GetAllSession()) {
        sessions_resp.emplace_back(*session);
    }

    std::vector<serialization::PlayersRepr> players_resp;
    for (const auto& pair : player_tokens_.GetPlayerToken()) {
        const Token& token = pair.first;
        auto player_id = pair.second->GetPlayerId();
        players_resp.emplace_back(player_id, token);
    }

    std::filesystem::path temp_path = game_save_path_.value();
    temp_path += ".tmp";

    try {
        std::fstream output_fstream;
        output_fstream.open(temp_path, std::ios_base::out);
        {
            boost::archive::text_oarchive oarchive{output_fstream};
            oarchive << sessions_resp << players_resp;
        }
        output_fstream.close();
        std::filesystem::rename(temp_path, game_save_path_.value());
    } catch (const std::exception& e) {
        std::filesystem::remove(temp_path);
    }
}

void Application::HandleRetiredPlayers(std::vector<domain::RetiredPlayers> retired_players) {
    use_cases_.AddRetiredPlayers(retired_players);

    for (const auto& retired_player : retired_players) {
        uint32_t player_id = retired_player.GetPlayerId();
        player_tokens_.RemovePlayerById(player_id);
        auto it = std::remove_if(players_.begin(), players_.end(),
                                 [player_id](const std::shared_ptr<Player>& player) {
            return player->GetPlayerId() == player_id;
        });
        players_.erase(it, players_.end());
    }
    retired_players.clear();
}

void Application::ConnectGameSessionSignals(std::shared_ptr<model::GameSession> session) {
    session->ConnectRetiredPlayersSignal(
        [this](std::vector<domain::RetiredPlayers> retired_players) mutable {
        HandleRetiredPlayers(std::move(retired_players));
    });
}

std::vector<domain::RetiredPlayers> Application::GetTableRecords(size_t start, size_t maxItems) {
    return use_cases_.GetTableRecords(start, maxItems);
}

} //namespace app
