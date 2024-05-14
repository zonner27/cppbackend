#pragma once
#include "http_server.h"
#include "model.h"

#include <boost/json.hpp>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;

//// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
////// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
namespace json = boost::json;

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
                mapObject["id"] = json::string(*map.GetId());
                mapObject["name"] = json::string(map.GetName());
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
            //std::cout << "mapId = " << mapIdstr << std::endl;

            model::Map::Id mapId = model::Map::Id(mapIdstr);
            const model::Map* map = game_.FindMap(mapId);

            if (map != nullptr) {
                json::object mapJson;
                mapJson["id"] = json::string(*map->GetId());
                mapJson["name"] = json::string(map->GetName());

                json::array roadsJsonArray;
                for (const auto& road : map->GetRoads()) {
                    json::object roadJson;
                    roadJson["x0"] = road.GetStart().x;
                    roadJson["y0"] = road.GetStart().y;
                    if (road.IsHorizontal()) {
                        roadJson["x1"] = road.GetEnd().x;
                    } else {
                        roadJson["y1"] = road.GetEnd().y;
                    }
                    roadsJsonArray.push_back(std::move(roadJson));
                }
                mapJson["roads"] = std::move(roadsJsonArray);

                json::array buildingsJsonArray;
                for (const auto& building : map->GetBuildings()) {
                    json::object buildingJson;
                    buildingJson["x"] = building.GetBounds().position.x;
                    buildingJson["y"] = building.GetBounds().position.y;
                    buildingJson["w"] = building.GetBounds().size.width;
                    buildingJson["h"] = building.GetBounds().size.height;
                    buildingsJsonArray.push_back(std::move(buildingJson));
                }
                mapJson["buildings"] = std::move(buildingsJsonArray);

                json::array officesJsonArray;
                for (const auto& office : map->GetOffices()) {
                    json::object officeJson;
                    officeJson["id"] = json::string(*office.GetId());
                    officeJson["x"] = office.GetPosition().x;
                    officeJson["y"] = office.GetPosition().y;
                    officeJson["offsetX"] = office.GetOffset().dx;
                    officeJson["offsetY"] = office.GetOffset().dy;
                    officesJsonArray.push_back(std::move(officeJson));
                }
                mapJson["offices"] = std::move(officesJsonArray);

                StringResponse response;
                response.result(http::status::ok);
                response.set(http::field::content_type, "application/json");
                response.body() = boost::json::serialize(mapJson);
                send(std::move(response));
                return;

            } else {
                //std::cout << "Map not found." << std::endl;
                boost::json::object errorObject;
                errorObject["code"] = "mapNotFound";
                errorObject["message"] = "Map not found";

                StringResponse response;
                response.result(http::status::not_found);
                response.set(http::field::content_type, "application/json");
                response.body() = boost::json::serialize(errorObject);
                send(std::move(response));
                return;
            }

         }

         boost::json::object errorObject;
         errorObject["code"] = "badRequest";
         errorObject["message"] = "Bad request";

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
