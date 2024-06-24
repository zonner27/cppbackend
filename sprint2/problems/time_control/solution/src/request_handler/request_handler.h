#pragma once
#include "../http_server/http_server.h"
#include "../model/model.h"
#include "../model/game.h"
#include "../model/game_session.h"
#include "../model/dog.h"
#include "../constants.h"
#include "../files.h"
#include "../streamadapter.h"
#include "../logger/logger.h"
#include "../app/players.h"
#include "../app/player_tokens.h"
#include "../app/application.h"

#include <boost/json.hpp>
#include <boost/beast.hpp>

#include <thread> //del
#include <chrono> //del

namespace http_handler {

namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;
namespace sys = boost::system;
using namespace std::literals;
using StringResponse = http::response<http::string_body>;
using StringRequest = http::request<http::string_body>;

class BaseRequestHandler {
public:
    explicit BaseRequestHandler(app::Application& application, model::Game& game, fs::path static_path)
        : application_{application}, game_{game}, static_path_{static_path} {}

    BaseRequestHandler(const BaseRequestHandler&) = delete;
    BaseRequestHandler& operator=(const BaseRequestHandler&) = delete;

protected:    
    model::Game& game_;
    fs::path static_path_;
    app::Application& application_;


    template <typename Send>
    void sendJsonResponse(const json::value& jsonResponse, Send&& send) {
        StringResponse response;
        response.result(http::status::ok);
        response.set(http::field::content_type, "application/json");    
        std::string body = boost::json::serialize(jsonResponse);
        response.body() = body;
        response.content_length(body.size());
        response.set(http::field::cache_control, "no-cache");

        SyncWriteOStreamAdapter adapter{std::cout}; ///del
        http::write(adapter, response);     //del

        send(std::move(response));
    }

    template <typename Send>
    void sendErrorResponse(const std::string& code, const std::string& message, http::status status, Send&& send,
                           const std::string& allowMethods = "GET, HEAD, OPTIONS") {
        boost::json::object errorObject;
        errorObject["code"] = code;
        errorObject["message"] = message;

        StringResponse response;
        response.result(status);
        response.set(http::field::content_type, "application/json");        
        if (status == http::status::method_not_allowed) {
            response.set(http::field::allow, allowMethods);
        }
        std::string body = boost::json::serialize(errorObject);
        response.body() = body;
        response.content_length(body.size());
        response.set(http::field::cache_control, "no-cache");

        SyncWriteOStreamAdapter adapter{std::cout}; ///del
        http::write(adapter, response);     //del

        send(std::move(response));
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

class ApiRequestHandler : public BaseRequestHandler, public std::enable_shared_from_this<ApiRequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    explicit ApiRequestHandler(app::Application& application, model::Game& game, fs::path static_path,  Strand& api_strand, app::PlayerTokens& playerTokens, app::Players& players) //net::io_context& ioc,
            : BaseRequestHandler(application, game, static_path), api_strand_{api_strand}, playerTokens_{playerTokens}, players_{players} {}

    using BaseRequestHandler::BaseRequestHandler;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        net::dispatch(api_strand_, [self = shared_from_this(), req = std::move(req), send = std::forward<Send>(send)]() mutable {
            if (req.target() == "/api/v1/maps" && req.method() == http::verb::get) {
                self->handleGetMapsRequest(std::forward<Send>(send));
            } else if (req.method() == http::verb::get && req.target().starts_with("/api/v1/maps/")) {
                self->handleGetMapByIdRequest(req.target().to_string().substr(13), std::forward<Send>(send));
            } else if (req.target() == "/api/v1/game/players" ) {
                self->handleGetPlayersRequest(req, std::forward<Send>(send));
            } else if (req.target() == "/api/v1/game/join") {
                self->handleJoinGameRequest(req, std::forward<Send>(send));
            } else if (req.target() == "/api/v1/game/state") {
                self->handleGetState(req, std::forward<Send>(send));
            } else if (req.target() == "/api/v1/game/player/action") {
                self->handleSetPlayerAction(req, std::forward<Send>(send));
            } else if (req.target() == "/api/v1/game/tick") {
                self->handleSetPlayersTick(req, std::forward<Send>(send));
            } else {
                self->sendErrorResponse("badRequest", "Bad request", http::status::bad_request, std::forward<Send>(send));
            }
        });
    }

private:
    Strand& api_strand_;
    app::PlayerTokens& playerTokens_;
    app::Players& players_;

    template <typename Send>
    void handleGetMapsRequest(Send&& send) {
        json::array jsonArray;

        for (const auto& map : game_.GetMaps()) {
            json::object mapObject;
            mapObject["id"] = json::string(*map.GetId());
            mapObject["name"] = json::string(map.GetName());
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
    void handleJoinGameRequest(const http::request<http::string_body>& req, Send&& send) {
        if (req.method() != http::verb::post) {
            const std::string allowedMethods = "POST";
            sendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        if (req[http::field::content_type] != "application/json") {
            sendErrorResponse("invalidArgument", "Invalid Content-Type", http::status::bad_request, std::forward<Send>(send));
            return;
        }

        try {
            auto body = json::parse(req.body());
            std::string userName = body.at("userName").as_string().c_str();
            std::string mapId = body.at("mapId").as_string().c_str();

            model::Map::Id mapIdObj{mapId};
            const model::Map* map = game_.FindMap(mapIdObj);

            if (userName.empty()) {
                sendErrorResponse("invalidArgument", "Invalid name", http::status::bad_request, std::forward<Send>(send));
                return;
            }

            if (!map) {
                sendErrorResponse("mapNotFound", "Map not found", http::status::not_found, std::forward<Send>(send));
                return;
            }

            if (players_.FindByDogNameAndMapId(userName, mapId) != nullptr) {
                sendErrorResponse("invalidArgument", "User with the same dog name on same map exists", http::status::bad_request, std::forward<Send>(send));
                return;
            }

//            std::shared_ptr<model::Dog> dog = std::make_shared<model::Dog>(userName);
//            std::shared_ptr<model::GameSession> validSession = game_.FindValidSession(map);
//            validSession->AddDog(dog);
//            app::Player& player = players_.Add(dog, validSession);
//            app::Token authToken = playerTokens_.AddPlayer(player);
//            uint32_t playerId = player.GetPlayerId();

//            app::Token authToken;
//            Player::ID playerId;

            auto [authToken, playerId] = application_.JoinGame(userName, map);

            json::object responseBody = {
                {"authToken", *authToken},
                {"playerId", playerId}
            };

            sendJsonResponse(responseBody, std::forward<Send>(send));

        } catch (const std::exception& e) {
            sendErrorResponse("invalidArgument", "Join game request parse error", http::status::bad_request, std::forward<Send>(send));
        }
    }

    template <typename Send>
    void handleGetPlayersRequest(const http::request<http::string_body>& req, Send&& send) {
        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            const std::string allowedMethods = "GET, HEAD";
            sendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        ExecuteAuthorized(req, std::forward<Send>(send), [this, &send](const app::Token& token) {

            //auto player = playerTokens_.FindPlayerByToken(token);
            auto player = application_.GetPlayerTokens().FindPlayerByToken(token);
            std::cout << "player name = " << player->GetDog()->GetName() << std::endl;
            std::shared_ptr<model::GameSession> player_session = player->GetSession();
            boost::json::object response_json;

            for (const std::shared_ptr<model::Dog>& dog : player_session->GetDogs()) {
                boost::json::object dog_json;
                dog_json["name"] = dog->GetName();
                response_json[std::to_string(dog->GetId())] = dog_json;
            }

            sendJsonResponse(response_json, std::forward<Send>(send));
        });
    }

    template <typename Send>
    void handleSetPlayerAction(const http::request<http::string_body>& req, Send&& send) {
        if (req.method() != http::verb::post) {
            const std::string allowedMethods = "POST";
            sendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        if (req[http::field::content_type] != "application/json") {
            sendErrorResponse("invalidArgument", "Invalid Content-Type", http::status::bad_request, std::forward<Send>(send));
            return;
        }

        ExecuteAuthorized(req, std::forward<Send>(send), [this, &req, &send](const app::Token& token) {

            try {
                json::value parsedJson = json::parse(req.body());
                json::object obj = parsedJson.as_object();

                if (!obj.contains("move") || !obj["move"].is_string()) {
                    sendErrorResponse("invalidArgument", "Failed to parse action", http::status::bad_request, std::forward<Send>(send));
                    return;
                }

                std::string move = obj["move"].as_string().c_str();

                //app::Player* player = playerTokens_.FindPlayerByToken(token);
                auto player = application_.GetPlayerTokens().FindPlayerByToken(token);

                std::shared_ptr<model::Dog> dog = player->GetDog();
//                std::cout << " dog count " << dog.use_count() << std::endl;
                const model::Map* map = player->GetSession()->GetMap();
                double s = map->GetDogSpeed();
//                std::pair<double, double> dogSpeed_tmp = dog->GetSpeed();
//                std::cout << "Speed = " << s << " for dog id " << dog->GetId() << " name " << dog->GetName() << " dog speed = " << std::to_string(dogSpeed_tmp.first) << " "
//                                                                            << std::to_string(dogSpeed_tmp.second) << std::endl;

                if (move == "L") {
                    dog->SetDirection(constants::Direction::WEST);
                    dog->SetSpeed({-s, 0});
                } else if (move == "R") {
                    dog->SetDirection(constants::Direction::EAST);
                    dog->SetSpeed({s, 0});
                } else if (move == "U") {
                    dog->SetDirection(constants::Direction::NORTH);
                    dog->SetSpeed({0, -s});
                } else if (move == "D") {
                    dog->SetDirection(constants::Direction::SOUTH);
                    dog->SetSpeed({0, s});
                } else if (move == "") {
                    //dog->SetDirection(constants::Direction::STOP);
                    dog->SetSpeed({0, 0});
                } else {
                    sendErrorResponse("invalidArgument", "Invalid move value", http::status::bad_request, std::forward<Send>(send));
                    return;
                }

//                dogSpeed_tmp = dog->GetSpeed();
//                std::cout << "After Speed = " << s << " for dog id " << dog->GetId() << " name " << dog->GetName() << " dog speed = " << std::to_string(dogSpeed_tmp.first) << " "
//                                                    << std::to_string(dogSpeed_tmp.second) << std::endl;;

                sendJsonResponse("{}", std::forward<Send>(send));

            } catch (const std::exception& e) {
                sendErrorResponse("invalidArgument", "Failed to parse action", http::status::bad_request, std::forward<Send>(send));
            }

        });
    }

    template <typename Send>
    void handleGetState(const http::request<http::string_body>& req, Send&& send) {
        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            const std::string allowedMethods = "GET, HEAD";
            sendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        ExecuteAuthorized(req, std::forward<Send>(send), [this, &send](const app::Token& token) {
            boost::json::object players_json;

            //std::vector<std::shared_ptr<model::GameSession>> sessions = game_.GetAllSession();
            std::vector<std::shared_ptr<model::GameSession>> sessions = application_.GetGame().GetAllSession();
            for (std::shared_ptr<model::GameSession>& session : sessions) {
                for (const std::shared_ptr<model::Dog>& dog : session->GetDogs()) {
                    boost::json::object dog_json;
                    dog_json["pos"] = {dog->GetCoordinate().x, dog->GetCoordinate().y};
                    dog_json["speed"] = {dog->GetSpeed().first, dog->GetSpeed().second};

                    //std::cout << std::fixed << std::setprecision(4); //del
                    //std::pair<double, double> dogSpeed_tmp = dog->GetSpeed();
                    //std::cout << "Speed = " << " for dog id " << dog->GetId() << " Name " << dog->GetName() << " dog speed = " << std::to_string(dogSpeed_tmp.first) << " "
                    //                         << std::to_string(dogSpeed_tmp.second) << std::endl;
                    switch (dog->GetDirection()) {
                        case constants::Direction::NORTH:
                            dog_json["dir"] = "U";
                            break;
                        case constants::Direction::WEST:
                            dog_json["dir"] = "L";
                            break;
                        case constants::Direction::EAST:
                            dog_json["dir"] = "R";
                            break;
                        case constants::Direction::SOUTH:
                            dog_json["dir"] = "D";
                            break;
                        case constants::Direction::STOP:
                            dog_json["dir"] = "";
                            break;
                        default:
                            dog_json["dir"] = "Unknown";
                            break;
                    }
                    players_json[std::to_string(dog->GetId())] = dog_json;
                }
            }
            json::object response_json;
            response_json["players"] = players_json;
            sendJsonResponse(response_json, std::forward<Send>(send));

        });
    }

    template <typename Send>
    void handleSetPlayersTick(const http::request<http::string_body>& req, Send&& send) {

        if (req.method() != http::verb::post) {
            const std::string allowedMethods = "POST";
            sendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        if (req[http::field::content_type] != "application/json") {
            sendErrorResponse("invalidArgument", "Invalid Content-Type", http::status::bad_request, std::forward<Send>(send));
            return;
        }

        try {

            json::value parsedJson = json::parse(req.body());
            json::object obj = parsedJson.as_object();

            if (!obj.contains("timeDelta") || !obj["timeDelta"].is_int64()) {
                sendErrorResponse("invalidArgument", "Failed to parse action", http::status::bad_request, std::forward<Send>(send));
                return;
            }

            int time_delta = obj["timeDelta"].as_int64();
            //std::cout << "time_delta = " << time_delta << std::endl;

            //std::vector<std::shared_ptr<model::GameSession>> sessions = game_.GetAllSession();
            std::vector<std::shared_ptr<model::GameSession>> sessions = application_.GetGame().GetAllSession();
            for (std::shared_ptr<model::GameSession>& session : sessions) {
                session->SetDogsCoordinatsByTime(time_delta);
            }


        } catch (const std::exception& e) {
            sendErrorResponse("invalidArgument", "Failed to parse action", http::status::bad_request, std::forward<Send>(send));
        }
        json::object response_json = {};

        sendJsonResponse(response_json, std::forward<Send>(send));
    }

    template <typename Fn, typename Send>
    void ExecuteAuthorized(const http::request<http::string_body>& req, Send&& send, Fn&& action) {
        auto authHeader = req.find(http::field::authorization);

        if (authHeader == req.end()) {
            sendErrorResponse("invalidToken", "Authorization header is required", http::status::unauthorized, std::forward<Send>(send));
            return;
        }

        std::string tokenStr = authHeader->value().to_string();
        const std::string bearerPrefix = "Bearer ";
        if (tokenStr.rfind(bearerPrefix, 0) == 0) {
            tokenStr = tokenStr.substr(bearerPrefix.size());
        } else {
            sendErrorResponse("invalidToken", "Invalid token format", http::status::unauthorized, std::forward<Send>(send));
            return;
        }

        if (tokenStr.size() != 32) {
            sendErrorResponse("invalidToken", "Invalid token format", http::status::unauthorized, std::forward<Send>(send));
            return;
        }

        app::Token token(tokenStr);
        //auto player = playerTokens_.FindPlayerByToken(token);
        auto player = application_.GetPlayerTokens().FindPlayerByToken(token);
        if (!player) {
            sendErrorResponse("unknownToken", "Player token has not been found", http::status::unauthorized, std::forward<Send>(send));
            return;
        }

        action(token);
    }

    json::object createMapJson(const model::Map& map);
};

class StaticFileRequestHandler : public BaseRequestHandler {
public:
    using BaseRequestHandler::BaseRequestHandler;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (req.method() == http::verb::get || req.method() == http::verb::head) {
            handleStaticFileRequest(std::move(req), std::forward<Send>(send));
        }
    }

private:
    template <typename Body, typename Allocator, typename Send>
    void handleStaticFileRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
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
            //std::this_thread::sleep_for(std::chrono::milliseconds(10)); //del
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
                {"ip"s, client_ip},
                {"URI"s, req.target().to_string()},
                {"method"s, req.method_string().to_string()}
        };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "request received"sv;
    }

    template <typename Body>
    void LogResponse(const http::response<Body>& resp, long long response_time, const std::string& client_ip) {

        json::value custom_data = json::object{
                {"ip"s, client_ip},
                {"response_time"s, response_time},
                {"code"s, resp.result_int()},
                {"content_type"s, resp.has_content_length() ? resp[http::field::content_type].to_string() : ""s}

        };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "response sent"sv;
    }

    SomeRequestHandler& decorated_;
};

}
