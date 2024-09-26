#include "api_request_handler.h"

namespace http_handler {

std::map<std::string, std::string> ApiRequestHandler::ParseQuery(const std::string &query) {
    std::map<std::string, std::string> query_map;
    std::istringstream query_stream(query);
    std::string param;

    while (std::getline(query_stream, param, '&')) {
        auto delimiter_pos = param.find('=');
        std::string key = param.substr(0, delimiter_pos);
        std::string value = param.substr(delimiter_pos + 1);
        query_map[key] = value;
    }

    return query_map;
}

json::object ApiRequestHandler::CreateMapJson(const model::Map& map) {

    json::object mapJson;
    mapJson[constants::ID] = json::string(*map.GetId());
    mapJson[constants::NAME] = json::string(map.GetName());

    json::array roadsJsonArray;
    for (const auto& road : map.GetRoads()) {
        roadsJsonArray.push_back(std::move(SerializeRoad(road)));
    }
    mapJson[constants::ROADS] = std::move(roadsJsonArray);

    json::array buildingsJsonArray;
    for (const auto& building : map.GetBuildings()) {
        buildingsJsonArray.push_back(std::move(SerializeBuilding(building)));
    }
    mapJson[constants::BUILDINGS] = std::move(buildingsJsonArray);

    json::array officesJsonArray;
    for (const auto& office : map.GetOffices()) {
        officesJsonArray.push_back(std::move(SerializeOffice(office)));
    }
    mapJson[constants::OFFICES] = std::move(officesJsonArray);

    json::array loot_typeJsonArray;
    for (const auto& loot_type : map.GetLootTypes()) {
        loot_typeJsonArray.push_back(std::move(SerializeLootType(loot_type)));
    }
    mapJson[constants::LOOT_TYPES] = std::move(loot_typeJsonArray);

    return mapJson;
}

json::object ApiRequestHandler::SerializeRoad(const model::Road &road) {

    json::object roadJson;
    roadJson[constants::X0] = road.GetStart().x;
    roadJson[constants::Y0] = road.GetStart().y;
    if (road.IsHorizontal()) {
        roadJson[constants::X1] = road.GetEnd().x;
    } else {
        roadJson[constants::Y1] = road.GetEnd().y;
    }
    return roadJson;
}

json::object ApiRequestHandler::SerializeBuilding(const model::Building &building) {

    json::object buildingJson;
    buildingJson[constants::X] = building.GetBounds().position.x;
    buildingJson[constants::Y] = building.GetBounds().position.y;
    buildingJson[constants::W] = building.GetBounds().size.width;
    buildingJson[constants::H] = building.GetBounds().size.height;
    return buildingJson;
}

json::object ApiRequestHandler::SerializeOffice(const model::Office &office) {

    json::object officeJson;
    officeJson[constants::ID] = json::string(*office.GetId());
    officeJson[constants::X] = office.GetPosition().x;
    officeJson[constants::Y] = office.GetPosition().y;
    officeJson[constants::OFFSET_X] = office.GetOffset().dx;
    officeJson[constants::OFFSET_Y] = office.GetOffset().dy;
    return officeJson;
}

json::object ApiRequestHandler::SerializeLootType(const model::LootType &loot_type) {
    json::object lootJson;
    if (!loot_type.name.empty()) {
        lootJson[constants::NAME] = loot_type.name;
    }
    if (!loot_type.file.empty()) {
        lootJson[constants::FILE] = loot_type.file;
    }
    if (!loot_type.type.empty()) {
        lootJson[constants::TYPE] = loot_type.type;
    }
    if (loot_type.rotation != std::numeric_limits<int>::min()) {
        lootJson[constants::ROTATION] = loot_type.rotation;
    }
    if (!loot_type.color.empty()) {
        lootJson[constants::COLOR] = loot_type.color;
    }
    if (!std::isnan(loot_type.scale)) {
        lootJson[constants::SCALE] = loot_type.scale;
    }
    if (loot_type.value != std::numeric_limits<int>::min()) {
        lootJson[constants::VALUE] = loot_type.value;
    }
    return lootJson;
}

} //namespace http_handler
