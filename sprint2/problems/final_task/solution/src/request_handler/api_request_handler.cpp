#include "api_request_handler.h"

namespace http_handler {

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

} //namespace http_handler
