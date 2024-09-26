#pragma once

#include <vector>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <functional>
#include <filesystem>
#include <fstream>

#include "players.h"
#include "player_tokens.h"
#include "../model/game.h"
#include "../time/ticker.h"

#include "../serialization/game_session_serialization.h"
#include "../serialization/player_serialization.h"
#include "../database/postgres.h"
#include "../database/db_settings.h"
#include "../database/use_cases_impl.h"

namespace app {

namespace net = boost::asio;
using namespace std::literals;
namespace fs = std::filesystem;

class Application : public std::enable_shared_from_this<Application> {

public:
    using Strand = net::strand<net::io_context::executor_type>;

    Application(model::Game& game, net::io_context& ioc, uint32_t tick_period,
                bool randomize_spawn_points, const db_app::DBSettings& db_settings) :
        game_{game},
        tick_period_{tick_period},
        randomize_spawn_points_{randomize_spawn_points},
        ioc_{ioc},
        api_strand_{std::make_shared<Strand>(net::make_strand(ioc))},
        db_{std::move(db_settings)} {
    }

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    std::pair<Token, Player::ID> JoinGame(std::string userName, const model::Map* map);
    Player* FindByDogNameAndMapId(const std::string& dogName, const std::string& mapId);
    PlayerTokens& GetPlayerTokens();
    model::Game& GetGame();
    std::shared_ptr<Strand> GetStrand();
    std::chrono::milliseconds GetTickPeriod();
    bool GetRandomizeSpawnPoints();
    void LoadGame(fs::path game_save_path, std::chrono::milliseconds save_period);
    void LoadGameFromArchive();
    void SaveGameByTime(const std::chrono::milliseconds& delta_time);
    void SaveGame();
    void HandleRetiredPlayers(std::vector<domain::RetiredPlayers> retired_players);
    void ConnectGameSessionSignals(std::shared_ptr<model::GameSession> session);
    std::vector<domain::RetiredPlayers> GetTableRecords(size_t start, size_t maxItems);

private:
    model::Game game_;
    std::chrono::milliseconds tick_period_;
    bool randomize_spawn_points_ = false;
    net::io_context& ioc_;
    std::shared_ptr<Strand> api_strand_;
    std::vector<std::shared_ptr<Player>> players_;
    PlayerTokens player_tokens_;
    std::shared_ptr<time_tiker::Ticker> ticker_;
    std::optional<fs::path> game_save_path_;
    std::chrono::milliseconds save_period_;
    std::shared_ptr<time_tiker::Ticker> save_ticker_;
    postgres::Database db_;
    db_app::UseCasesImpl use_cases_{db_.GetRetiredPlayers()};
};

} //namespace app
