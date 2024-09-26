#include "use_cases_impl.h"

#include <boost/uuid/uuid_io.hpp>
#include <string>
#include <sstream>

namespace db_app {
using namespace domain;

void UseCasesImpl::AddRetiredPlayers(const std::vector<domain::RetiredPlayers> &retired_players) {
    retired_players_.SaveRetiredPlayers(retired_players);
}

std::vector<RetiredPlayers> UseCasesImpl::GetTableRecords(size_t start, size_t maxItems) {
    return retired_players_.GetTableRecord(start, maxItems);
}

}  // namespace app
