#include "request_handler.h"
#include "api_request_handler.h"

namespace http_handler {

json::object ApiRequestHandler::createMapJson(const model::Map& map) {
    json::object mapJson;
    mapJson[constants::ID] = json::string(*map.GetId());
    mapJson[constants::NAME] = json::string(map.GetName());

    json::array roadsJsonArray;
    for (auto it = map.GetRoads().begin(); it != map.GetRoads().end(); ++it) {

        json::object roadJson;
        roadJson[constants::X0] = it->GetStart().x;
        roadJson[constants::Y0] = it->GetStart().y;
        if (it->IsHorizontal()) {
            roadJson[constants::X1] = it->GetEnd().x;
        } else {
            roadJson[constants::Y1] = it->GetEnd().y;
        }
        roadsJsonArray.push_back(std::move(roadJson));
    }
    mapJson[constants::ROADS] = std::move(roadsJsonArray);

    json::array buildingsJsonArray;
    for (const auto& building : map.GetBuildings()) {
        json::object buildingJson;
        buildingJson[constants::X] = building.GetBounds().position.x;
        buildingJson[constants::Y] = building.GetBounds().position.y;
        buildingJson[constants::W] = building.GetBounds().size.width;
        buildingJson[constants::H] = building.GetBounds().size.height;
        buildingsJsonArray.push_back(std::move(buildingJson));
    }
    mapJson[constants::BUILDINGS] = std::move(buildingsJsonArray);

    json::array officesJsonArray;
    for (const auto& office : map.GetOffices()) {
        json::object officeJson;
        officeJson[constants::ID] = json::string(*office.GetId());
        officeJson[constants::X] = office.GetPosition().x;
        officeJson[constants::Y] = office.GetPosition().y;
        officeJson[constants::OFFSET_X] = office.GetOffset().dx;
        officeJson[constants::OFFSET_Y] = office.GetOffset().dy;
        officesJsonArray.push_back(std::move(officeJson));
    }
    mapJson[constants::OFFICES] = std::move(officesJsonArray);

    return mapJson;
}


}  // namespace http_handler
