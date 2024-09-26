#pragma once
#include "use_cases.h"


namespace db_app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::RetiredPlayersRepository& retired_players)
        : retired_players_{retired_players} {
    }    

    void AddRetiredPlayers(const std::vector<domain::RetiredPlayers>& retired_players) override;
    std::vector<domain::RetiredPlayers> GetTableRecords(size_t start, size_t maxItems) override;
private:
    domain::RetiredPlayersRepository& retired_players_;
};

}  // namespace db_app
