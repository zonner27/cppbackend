#pragma once
#include "http_server.h"
#include "model.h"
#include "constants.h"

#include <boost/json.hpp>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

using StringResponse = http::response<http::string_body>;

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

         if (req.target() == "/api/v1/maps" && req.method() == http::verb::get) {

            json::array jsonArray;

            for (const auto& map : game_.GetMaps()) {
                json::object mapObject;
                mapObject[constants::ID] = json::string(*map.GetId());
                mapObject[constants::NAME] = json::string(map.GetName());
                jsonArray.push_back(std::move(mapObject));
            }

            StringResponse response;
            response.result(http::status::ok);
            response.set(http::field::content_type, "application/json");
            response.body() = boost::json::serialize(jsonArray);
            send(std::move(response));
            return;
         }

         if (req.method() == http::verb::get && req.target().starts_with("/api/v1/maps/")) {
            std::string mapIdstr = req.target().to_string().substr(13);

            model::Map::Id mapId = model::Map::Id(mapIdstr);
            const model::Map* map = game_.FindMap(mapId);

            if (map != nullptr) {
                json::object mapJson;
                mapJson[constants::ID] = json::string(*map->GetId());
                mapJson[constants::NAME] = json::string(map->GetName());

                json::array roadsJsonArray;
                for (const auto& road : map->GetRoads()) {
                    json::object roadJson;
                    roadJson[constants::X0] = road.GetStart().x;
                    roadJson[constants::Y0] = road.GetStart().y;
                    if (road.IsHorizontal()) {
                        roadJson[constants::X1] = road.GetEnd().x;
                    } else {
                        roadJson[constants::Y1] = road.GetEnd().y;
                    }
                    roadsJsonArray.push_back(std::move(roadJson));
                }
                mapJson[constants::ROADS] = std::move(roadsJsonArray);

                json::array buildingsJsonArray;
                for (const auto& building : map->GetBuildings()) {
                    json::object buildingJson;
                    buildingJson[constants::X] = building.GetBounds().position.x;
                    buildingJson[constants::Y] = building.GetBounds().position.y;
                    buildingJson[constants::W] = building.GetBounds().size.width;
                    buildingJson[constants::H] = building.GetBounds().size.height;
                    buildingsJsonArray.push_back(std::move(buildingJson));
                }
                mapJson[constants::BUILDINGS] = std::move(buildingsJsonArray);

                json::array officesJsonArray;
                for (const auto& office : map->GetOffices()) {
                    json::object officeJson;
                    officeJson[constants::ID] = json::string(*office.GetId());
                    officeJson[constants::X] = office.GetPosition().x;
                    officeJson[constants::Y] = office.GetPosition().y;
                    officeJson[constants::OFFSET_X] = office.GetOffset().dx;
                    officeJson[constants::OFFSET_Y] = office.GetOffset().dy;
                    officesJsonArray.push_back(std::move(officeJson));
                }
                mapJson[constants::OFFICES] = std::move(officesJsonArray);

                StringResponse response;
                response.result(http::status::ok);
                response.set(http::field::content_type, "application/json");
                response.body() = boost::json::serialize(mapJson);
                send(std::move(response));
                return;

            } else {
                boost::json::object errorObject;
                errorObject[constants::CODE] = "mapNotFound";
                errorObject[constants::MESSAGE] = "Map not found";

                StringResponse response;
                response.result(http::status::not_found);
                response.set(http::field::content_type, "application/json");
                response.body() = boost::json::serialize(errorObject);
                send(std::move(response));
                return;
            }
         }

         boost::json::object errorObject;
         errorObject[constants::CODE] = "badRequest";
         errorObject[constants::MESSAGE] = "Bad request";

         StringResponse response;
         response.result(http::status::bad_request);
         response.set(http::field::content_type, "application/json");
         response.body() = boost::json::serialize(errorObject);
         send(std::move(response));
         return;
    }

private:
    model::Game& game_;
};

}
