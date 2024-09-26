#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "connection_pool.h"
#include "retired_players.h"
#include "db_settings.h"

namespace postgres {

class RetiredPlayersRepositoryImpl : public domain::RetiredPlayersRepository {
public:
    explicit RetiredPlayersRepositoryImpl(db_app::ConnectionPool& connection_pool)
        : connection_pool_{connection_pool} {
    }
    void SaveRetiredPlayers(const std::vector<domain::RetiredPlayers>& retired_players) override;
    std::vector<domain::RetiredPlayers> GetTableRecord(size_t start, size_t maxItems) override;

private:
    db_app::ConnectionPool& connection_pool_;
};

class Database {
public:
    explicit Database(const db_app::DBSettings& db_settings);

    RetiredPlayersRepositoryImpl& GetRetiredPlayers() & {
        return retired_players_;
    }

private:
    db_app::ConnectionPool conn_pool_;
    RetiredPlayersRepositoryImpl retired_players_{conn_pool_};
};

}  // namespace postgres
