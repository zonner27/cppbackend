#pragma once

#include <string>
#include <vector>
#include "retired_players.h"

namespace db_app {

class UseCases {
public:
    virtual void AddRetiredPlayers(const std::vector<domain::RetiredPlayers>& retired_players) = 0;
    virtual std::vector<domain::RetiredPlayers> GetTableRecords(size_t start, size_t maxItems) = 0;
protected:
    ~UseCases() = default;
};

}  // namespace db_app
