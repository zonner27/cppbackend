#pragma once

#include <filesystem>
#include <fstream>
#include <boost/json.hpp>

#include "../model/model.h"
#include "../model/game.h"
#include "../model/dog.h"
#include "../constants.h"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);
void ParseRoads(const boost::json::value& mapData, model::Map& map);
void ParseBuildings(const boost::json::value& mapData, model::Map& map);
void ParseOffices(const boost::json::value& mapData, model::Map& map);
void ParseLootTypes(const boost::json::value& mapData, model::Map& map);

}  // namespace json_loader
