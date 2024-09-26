#include "postgres.h"

#include <pqxx/zview.hxx>
#include <pqxx/pqxx>
#include "../constants.h"

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void RetiredPlayersRepositoryImpl::SaveRetiredPlayers(const std::vector<domain::RetiredPlayers>& retired_players) {
    auto conn = connection_pool_.GetConnection();
    pqxx::work work{*conn};
    for (const auto& player : retired_players) {
        work.exec_params(
            R"(
                INSERT INTO retired_players (id, name, score, play_time_ms) VALUES ($1, $2, $3, $4)
                ON CONFLICT (id) DO UPDATE SET name=$2;
            )"_zv,
            player.GetId().ToString(),
            player.GetName(),
            player.GetScore(),
            player.GetPlayTime());
    }
    work.commit();
}

std::vector<domain::RetiredPlayers> RetiredPlayersRepositoryImpl::GetTableRecord(size_t start, size_t maxItems){
    auto conn = connection_pool_.GetConnection();
    std::vector<domain::RetiredPlayers> table_records;
    pqxx::read_transaction read_transaction_{*conn};

    pqxx::result result = read_transaction_.exec_params(
        R"(
        SELECT name, score, play_time_ms
        FROM retired_players
        ORDER BY score DESC, play_time_ms, name
        LIMIT $1 OFFSET $2
        )", maxItems, start);

    for (const auto& row : result) {
        std::string name = row["name"].as<std::string>();
        uint32_t score = row["score"].as<uint32_t>();
        uint32_t play_time = row["play_time_ms"].as<uint32_t>();
        table_records.emplace_back(domain::RetiredPlayersId::New(), std::move(name), 0, score, play_time);
    }

    return table_records;
}

Database::Database(const db_app::DBSettings& db_settings)
    : conn_pool_{
          db_settings.number_connections,
          [db_url = std::move(db_settings.db_url)]() {
             return std::make_shared<pqxx::connection>(db_url);
         }
    } {
    auto conn = conn_pool_.GetConnection();
    pqxx::work work{*conn};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS retired_players (
    id UUID PRIMARY KEY,
    name varchar(100) NOT NULL,
    score INT,
    play_time_ms INT
);
CREATE INDEX IF NOT EXISTS idx_retired_players_score_playtime_name
ON retired_players (score DESC, play_time_ms, name);
)"_zv);
    // коммитим изменения
    work.commit();
}

}  // namespace postgres


