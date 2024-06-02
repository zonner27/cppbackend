#pragma once
#include "http_server.h"
#include "model.h"
#include "constants.h"
#include "files.h"
#include "streamadapter.h"
#include "logger.h"

#include <boost/json.hpp>
#include <boost/beast.hpp>


namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace sys = boost::system;
using namespace std::literals;
using StringResponse = http::response<http::string_body>;
using StringRequest = http::request<http::string_body>;

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

        } else {

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
                        StringResponse response;
                        response.result(http::status::bad_request);
                        response.set(http::field::content_type, "text/plain");
                        response.body() = "Invalid request: path is outside of the static directory\n";
                        send(std::move(response));
                        return;
                    }

                    if (fs::exists(file_path) && fs::is_regular_file(file_path)) {

                       std::string mime_types = files_path::mime_decode(file_path);
                       http::file_body::value_type file;

                       if (sys::error_code ec; file.open(file_path.string().c_str(), beast::file_mode::read, ec), ec) {
                           StringResponse error_response;
                           error_response.result(http::status::internal_server_error);
                           error_response.set(http::field::content_type, "text/plain");
                           error_response.body() = "Failed to open file" + ec.message();
                           error_response.prepare_payload();
                           send(std::move(error_response));
                           return;
                       }

                       http::response<http::file_body> response;
                       std::uint64_t file_size = fs::file_size(file_path);
                       response.result(http::status::ok);
                       response.set(http::field::content_type, mime_types);
                       response.set(http::field::content_length, std::to_string(file_size));
                       response.body() = std::move(file);
                       response.prepare_payload();

                       //SyncWriteOStreamAdapter adapter{std::cout};
                       //http::write(adapter, response);

                       send(std::move(response));
                       return;

                   } else {
                        StringResponse response;
                        response.result(http::status::not_found);
                        response.set(http::field::content_type, "text/plain");
                        response.body() = "Invalid request: File does not exist\n";
                        send(std::move(response));
                        return;
                   }
            }
        }
    }

private:
    model::Game& game_;
    fs::path static_path_;
};


template<class SomeRequestHandler>
class LoggingRequestHandler {

public:

    explicit LoggingRequestHandler(SomeRequestHandler& handler) : decorated_(handler) {}

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req,  const std::string& client_ip, Send&& send) {

        auto start = std::chrono::high_resolution_clock::now();
        LogRequest(req, client_ip);

        decorated_(std::move(req), [&](auto&& response) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            LogResponse(response, duration, client_ip);
            send(std::move(response));
        });
     }

private:
    template <typename Body, typename Allocator>
    static void LogRequest(const http::request<Body, http::basic_fields<Allocator>>& req, const std::string& client_ip) {

        json::value custom_data = json::object{
                {"ip", client_ip},
                {"URI", req.target().to_string()},
                {"method", req.method_string().to_string()}
        };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "request received";
    }

    template <typename Body>
    void LogResponse(const http::response<Body>& resp, long long response_time, const std::string& client_ip) {
        json::value custom_data = json::object{
                {"ip", client_ip},
                {"response_time", response_time},
                {"code", resp.result_int()},
                {"content_type", resp.has_content_length() ? resp[http::field::content_type].to_string() : ""}

        };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "response sent";
    }

    SomeRequestHandler& decorated_;
    const net::ip::tcp::endpoint endpoint_;
};

}
