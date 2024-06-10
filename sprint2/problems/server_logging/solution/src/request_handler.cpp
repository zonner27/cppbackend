#include "request_handler.h"

namespace http_handler {

json::object ApiRequestHandler::createMapJson(const model::Map& map) {
    json::object mapJson;
    mapJson[constants::ID] = json::string(*map.GetId());
    mapJson[constants::NAME] = json::string(map.GetName());

    json::array roadsJsonArray;
    for (const auto& road : map.GetRoads()) {
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

/*
class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, fs::path static_path)
        : game_{game}, static_path_{static_path} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

        if (req.target().starts_with("/api/")) {
            handleApiRequest(std::move(req), std::forward<Send>(send));
        } else {
            handleStaticFileRequest(std::move(req), std::forward<Send>(send));
        }
    }

private:
    model::Game& game_;
    fs::path static_path_;
    json::object createMapJson(const model::Map& map);

    template <typename Body, typename Allocator, typename Send>
    void handleApiRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (req.target() == "/api/v1/maps" && req.method() == http::verb::get) {
            handleGetMapsRequest(std::forward<Send>(send));
        } else if (req.method() == http::verb::get && req.target().starts_with("/api/v1/maps/")) {
            handleGetMapByIdRequest(req.target().to_string().substr(13), std::forward<Send>(send));
        } else {
            sendErrorResponse("badRequest", "Bad request", http::status::bad_request, std::forward<Send>(send));
        }
    }

    template <typename Send>
    void sendJsonResponse(const json::value& jsonResponse, Send&& send) {
        StringResponse response;
        response.result(http::status::ok);
        response.set(http::field::content_type, "application/json");
        response.body() = boost::json::serialize(jsonResponse);
        send(std::move(response));
    }

    template <typename Send>
    void handleGetMapsRequest(Send&& send) {
        json::array jsonArray;

        for (const auto& map : game_.GetMaps()) {
            json::object mapObject;
            mapObject[constants::ID] = json::string(*map.GetId());
            mapObject[constants::NAME] = json::string(map.GetName());
            jsonArray.push_back(std::move(mapObject));
        }
        sendJsonResponse(jsonArray, std::forward<Send>(send));
    }

    template <typename Send>
    void handleGetMapByIdRequest(const std::string& mapIdStr, Send&& send) {
        model::Map::Id mapId = model::Map::Id(mapIdStr);
        const model::Map* map = game_.FindMap(mapId);

        if (map != nullptr) {
            json::object mapJson = createMapJson(*map);
            sendJsonResponse(mapJson, std::forward<Send>(send));
        } else {
            sendErrorResponse("mapNotFound", "Map not found", http::status::not_found, std::forward<Send>(send));
        }
    }

    template <typename Send>
    void sendErrorResponse(const std::string& code, const std::string& message, http::status status, Send&& send) {
        boost::json::object errorObject;
        errorObject[constants::CODE] = code;
        errorObject[constants::MESSAGE] = message;

        StringResponse response;
        response.result(status);
        response.set(http::field::content_type, "application/json");
        response.body() = boost::json::serialize(errorObject);
        send(std::move(response));
    }

    template <typename Body, typename Allocator, typename Send>
    void handleStaticFileRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
       if (req.method() == http::verb::get || req.method() == http::verb::head) {
           std::string path_str = req.target().to_string();

           if (!path_str.empty() && path_str[0] == '/') {
               path_str = path_str.substr(1);
               path_str = files_path::url_decode(path_str);
           }

           if (path_str.empty()) {
               path_str = "index.html";
           }

           fs::path file_path = static_path_ / fs::path(path_str);

           if (!files_path::IsSubPath(file_path, static_path_)) {
               sendTextResponse("Invalid request: path is outside of the static directory\n", http::status::bad_request, std::forward<Send>(send));
               return;
           }

           if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
               sendFileResponse(file_path, std::forward<Send>(send));
           } else {
               sendTextResponse("Invalid request: File does not exist\n", http::status::not_found, std::forward<Send>(send));
           }
       }
    }

    template <typename Send>
    void sendTextResponse(const std::string& text, http::status status, Send&& send) {
       http::response<http::string_body> response;
       response.result(status);
       response.set(http::field::content_type, "text/plain");
       response.body() = text;
       response.prepare_payload();
       send(std::move(response));
    }

    template <typename Send>
    void sendFileResponse(fs::path& file_path, Send&& send) {
       std::string mime_types = files_path::mime_decode(file_path);
       http::file_body::value_type file;

       if (sys::error_code ec; file.open(file_path.string().c_str(), beast::file_mode::read, ec), ec) {
           sendTextResponse("Failed to open file: " + ec.message(), http::status::internal_server_error, std::forward<Send>(send));
           return;
       }

       http::response<http::file_body> response;
       response.result(http::status::ok);
       response.set(http::field::content_type, mime_types);
       response.content_length(file.size());
       response.body() = std::move(file);
       response.prepare_payload();
       send(std::move(response));
    }
};
*/


//        if (req.target().starts_with("/api/")) {

//            if (req.target() == "/api/v1/maps" && req.method() == http::verb::get) {

//                json::array jsonArray;

//                for (const auto& map : game_.GetMaps()) {
//                    json::object mapObject;
//                    mapObject[constants::ID] = json::string(*map.GetId());
//                    mapObject[constants::NAME] = json::string(map.GetName());
//                    jsonArray.push_back(std::move(mapObject));
//                }

//                StringResponse response;
//                response.result(http::status::ok);
//                response.set(http::field::content_type, "application/json");
//                response.body() = boost::json::serialize(jsonArray);
//                send(std::move(response));
//                return;
//            }

//            if (req.method() == http::verb::get && req.target().starts_with("/api/v1/maps/")) {
//               std::string mapIdstr = req.target().to_string().substr(13);

//               model::Map::Id mapId = model::Map::Id(mapIdstr);
//               const model::Map* map = game_.FindMap(mapId);

//               if (map != nullptr) {
//                   json::object mapJson;
//                   mapJson[constants::ID] = json::string(*map->GetId());
//                   mapJson[constants::NAME] = json::string(map->GetName());

//                   json::array roadsJsonArray;
//                   for (const auto& road : map->GetRoads()) {
//                       json::object roadJson;
//                       roadJson[constants::X0] = road.GetStart().x;
//                       roadJson[constants::Y0] = road.GetStart().y;
//                       if (road.IsHorizontal()) {
//                           roadJson[constants::X1] = road.GetEnd().x;
//                       } else {
//                           roadJson[constants::Y1] = road.GetEnd().y;
//                       }
//                       roadsJsonArray.push_back(std::move(roadJson));
//                   }
//                   mapJson[constants::ROADS] = std::move(roadsJsonArray);

//                   json::array buildingsJsonArray;
//                   for (const auto& building : map->GetBuildings()) {
//                       json::object buildingJson;
//                       buildingJson[constants::X] = building.GetBounds().position.x;
//                       buildingJson[constants::Y] = building.GetBounds().position.y;
//                       buildingJson[constants::W] = building.GetBounds().size.width;
//                       buildingJson[constants::H] = building.GetBounds().size.height;
//                       buildingsJsonArray.push_back(std::move(buildingJson));
//                   }
//                   mapJson[constants::BUILDINGS] = std::move(buildingsJsonArray);

//                   json::array officesJsonArray;
//                   for (const auto& office : map->GetOffices()) {
//                       json::object officeJson;
//                       officeJson[constants::ID] = json::string(*office.GetId());
//                       officeJson[constants::X] = office.GetPosition().x;
//                       officeJson[constants::Y] = office.GetPosition().y;
//                       officeJson[constants::OFFSET_X] = office.GetOffset().dx;
//                       officeJson[constants::OFFSET_Y] = office.GetOffset().dy;
//                       officesJsonArray.push_back(std::move(officeJson));
//                   }
//                   mapJson[constants::OFFICES] = std::move(officesJsonArray);

//                   StringResponse response;
//                   response.result(http::status::ok);
//                   response.set(http::field::content_type, "application/json");
//                   response.body() = boost::json::serialize(mapJson);
//                   send(std::move(response));
//                   return;

//               } else {
//                   boost::json::object errorObject;
//                   errorObject[constants::CODE] = "mapNotFound";
//                   errorObject[constants::MESSAGE] = "Map not found";

//                   StringResponse response;
//                   response.result(http::status::not_found);
//                   response.set(http::field::content_type, "application/json");
//                   response.body() = boost::json::serialize(errorObject);
//                   send(std::move(response));
//                   return;
//               }
//            }

//            boost::json::object errorObject;
//            errorObject[constants::CODE] = "badRequest";
//            errorObject[constants::MESSAGE] = "Bad request";

//            StringResponse response;
//            response.result(http::status::bad_request);
//            response.set(http::field::content_type, "application/json");
//            response.body() = boost::json::serialize(errorObject);
//            send(std::move(response));
//            return;

//        } else {

//            if (req.method() == http::verb::get || req.method() == http::verb::head) {

//                    std::string path_str = req.target().to_string();

//                    if (!path_str.empty() && path_str[0] == '/') {
//                        path_str = path_str.substr(1);
//                        path_str = files_path::url_decode(path_str);
//                    }

//                    if (path_str.empty()) {
//                        path_str = "index.html";
//                    }

//                    fs::path file_path = static_path_ / fs::path(path_str);

//                    if (!files_path::IsSubPath(file_path, static_path_)) {
//                        StringResponse response;
//                        response.result(http::status::bad_request);
//                        response.set(http::field::content_type, "text/plain");
//                        response.body() = "Invalid request: path is outside of the static directory\n";
//                        send(std::move(response));
//                        return;
//                    }

//                    if (fs::exists(file_path) && fs::is_regular_file(file_path)) {

//                       std::string mime_types = files_path::mime_decode(file_path);
//                       http::file_body::value_type file;

//                       if (sys::error_code ec; file.open(file_path.string().c_str(), beast::file_mode::read, ec), ec) {
//                           StringResponse error_response;
//                           error_response.result(http::status::internal_server_error);
//                           error_response.set(http::field::content_type, "text/plain");
//                           error_response.body() = "Failed to open file" + ec.message();
//                           error_response.prepare_payload();
//                           send(std::move(error_response));
//                           return;
//                       }

//                       http::response<http::file_body> response;
//                       std::uint64_t file_size = fs::file_size(file_path);
//                       response.result(http::status::ok);
//                       response.set(http::field::content_type, mime_types);
//                       response.set(http::field::content_length, std::to_string(file_size));
//                       response.body() = std::move(file);
//                       response.prepare_payload();

//                       //SyncWriteOStreamAdapter adapter{std::cout};
//                       //http::write(adapter, response);

//                       send(std::move(response));
//                       return;

//                   } else {
//                        StringResponse response;
//                        response.result(http::status::not_found);
//                        response.set(http::field::content_type, "text/plain");
//                        response.body() = "Invalid request: File does not exist\n";
//                        send(std::move(response));
//                        return;
//                   }
//            }
//        }
