#pragma once

#include "request_handler.h"
#include "../database/retired_players.h"

namespace http_handler {

class ApiRequestHandler : public BaseRequestHandler, public std::enable_shared_from_this<ApiRequestHandler> {
public:
    using BaseRequestHandler::BaseRequestHandler;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

        if (req.target() == "/api/v1/maps") {
            HandleGetMapsRequest(req, std::forward<Send>(send));
        } else if (req.target().starts_with("/api/v1/maps/")) {
            HandleGetMapByIdRequest(req, std::forward<Send>(send));
        } else if (req.target() == "/api/v1/game/players" ) {
            HandleGetPlayersRequest(req, std::forward<Send>(send));
        } else if (req.target() == "/api/v1/game/join") {
            HandleJoinGameRequest(req, std::forward<Send>(send));
        } else if (req.target() == "/api/v1/game/state") {
            HandleGetState(req, std::forward<Send>(send));
        } else if (req.target() == "/api/v1/game/player/action") {
            HandleSetPlayerAction(req, std::forward<Send>(send));
        } else if (req.target() == "/api/v1/game/tick") {
            HandleSetPlayersTick(req, std::forward<Send>(send));
        } else if (req.target().starts_with("/api/v1/game/records")) {
            HandleGetTableRecords(req, std::forward<Send>(send));
        } else {
            SendErrorResponse("badRequest", "Bad request", http::status::bad_request, std::forward<Send>(send));
        }
    }
private:

    template <typename Send>
    void HandleGetMapsRequest(const http::request<http::string_body>& req, Send&& send) {
        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            const std::string allowedMethods = "GET, HEAD";
            SendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        json::array jsonArray;

        for (const auto& map : application_.GetGame().GetMaps()) {
            json::object mapObject;
            mapObject["id"] = json::string(*map.GetId());
            mapObject["name"] = json::string(map.GetName());
            jsonArray.push_back(std::move(mapObject));
        }
        SendJsonResponse(jsonArray, std::forward<Send>(send));
    }

    template <typename Send>
    void HandleGetMapByIdRequest(const http::request<http::string_body>& req, Send&& send) {

        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            const std::string allowedMethods = "GET, HEAD";
            SendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        std::string mapIdStr = req.target().to_string().substr(13);
        model::Map::Id mapId = model::Map::Id(mapIdStr);
        const model::Map* map = application_.GetGame().FindMap(mapId);

        if (map != nullptr) {
            json::object mapJson = CreateMapJson(*map);
            SendJsonResponse(mapJson, std::forward<Send>(send));
        } else {
            SendErrorResponse("mapNotFound", "Map not found", http::status::not_found, std::forward<Send>(send));
        }
    }

    template <typename Send>
    void HandleJoinGameRequest(const http::request<http::string_body>& req, Send&& send) {

        if (req.method() != http::verb::post) {
            const std::string allowedMethods = "POST";
            SendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        if (req[http::field::content_type] != "application/json") {
            SendErrorResponse("invalidArgument", "Invalid Content-Type", http::status::bad_request, std::forward<Send>(send));
            return;
        }

        try {
            auto body = json::parse(req.body());
            std::string userName = body.at("userName").as_string().c_str();
            std::string mapId = body.at("mapId").as_string().c_str();

            model::Map::Id mapIdObj{mapId};
            const model::Map* map = application_.GetGame().FindMap(mapIdObj);

            if (userName.empty()) {
                SendErrorResponse("invalidArgument", "Invalid name", http::status::bad_request, std::forward<Send>(send));
                return;
            }

            if (!map) {
                SendErrorResponse("mapNotFound", "Map not found", http::status::not_found, std::forward<Send>(send));
                return;
            }

            if (application_.FindByDogNameAndMapId(userName, mapId) != nullptr) {
                SendErrorResponse("invalidArgument", "User with the same dog name on same map exists", http::status::bad_request, std::forward<Send>(send));
                return;
            }

            json::object responseBody;
            auto [authToken, playerId] = application_.JoinGame(userName, map);

            responseBody = {
                {"authToken", *authToken},
                {"playerId", playerId}
            };

            SendJsonResponse(responseBody, std::forward<Send>(send));

        } catch (const std::exception& e) {
            SendErrorResponse("invalidArgument", "Join game request parse error", http::status::bad_request, std::forward<Send>(send));
        }
    }

    template <typename Send>
    void HandleGetPlayersRequest(const http::request<http::string_body>& req, Send&& send) {

        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            const std::string allowedMethods = "GET, HEAD";
            SendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        ExecuteAuthorized(req, std::forward<Send>(send), [this, &send](const app::Token& token) {

            auto player = application_.GetPlayerTokens().FindPlayerByToken(token);
            std::weak_ptr<model::GameSession> player_session = player->GetSession();
            boost::json::object response_json;

            for (const std::weak_ptr<model::Dog>& dog : player_session.lock()->GetDogs()) {      //std::shared_ptr<model::Dog>&
                boost::json::object dog_json;
                dog_json["name"] = dog.lock()->GetName();
                response_json[std::to_string(dog.lock()->GetId())] = dog_json;
            }

            SendJsonResponse(response_json, std::forward<Send>(send));
        });
    }

    template <typename Send>
    void HandleSetPlayerAction(const http::request<http::string_body>& req, Send&& send) {

        if (req.method() != http::verb::post) {
            const std::string allowedMethods = "POST";
            SendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        if (req[http::field::content_type] != "application/json") {
            SendErrorResponse("invalidArgument", "Invalid Content-Type", http::status::bad_request, std::forward<Send>(send));
            return;
        }

        ExecuteAuthorized(req, std::forward<Send>(send), [this, &req, &send](const app::Token& token) {

            try {
                json::value parsedJson = json::parse(req.body());
                json::object obj = parsedJson.as_object();

                if (!obj.contains("move") || !obj["move"].is_string()) {
                    SendErrorResponse("invalidArgument", "Failed to parse action", http::status::bad_request, std::forward<Send>(send));
                    return;
                }

                std::string move = obj["move"].as_string().c_str();
                auto player = application_.GetPlayerTokens().FindPlayerByToken(token);

                std::weak_ptr<model::Dog> dog = player->GetDog();
                const model::Map* map = player->GetSession().lock()->GetMap();
                double speed = map->GetDogSpeed();

                net::dispatch(*player->GetSession().lock()->GetSessionStrand(), [self = shared_from_this(),
                              &move, &dog, &speed, req = std::move(req), send = std::forward<Send>(send)]() mutable {

                    if (move == "L") {
                        dog.lock()->SetDirection(constants::Direction::WEST);
                        dog.lock()->SetSpeed({-speed, 0});
                    } else if (move == "R") {
                        dog.lock()->SetDirection(constants::Direction::EAST);
                        dog.lock()->SetSpeed({speed, 0});
                    } else if (move == "U") {
                        dog.lock()->SetDirection(constants::Direction::NORTH);
                        dog.lock()->SetSpeed({0, -speed});
                    } else if (move == "D") {
                        dog.lock()->SetDirection(constants::Direction::SOUTH);
                        dog.lock()->SetSpeed({0, speed});
                    } else if (move == "") {
                        dog.lock()->SetSpeed({0, 0});
                    } else {
                        self->SendErrorResponse("invalidArgument", "Invalid move value", http::status::bad_request, std::forward<Send>(send));
                        return;
                    }

                });

                SendJsonResponse("{}", std::forward<Send>(send));

                } catch (const std::exception& e) {
                    SendErrorResponse("invalidArgument", "Failed to parse action", http::status::bad_request, std::forward<Send>(send));
                }
        });
    }

    template <typename Send>
    void HandleGetState(const http::request<http::string_body>& req, Send&& send) {

        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            const std::string allowedMethods = "GET, HEAD";
            SendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        ExecuteAuthorized(req, std::forward<Send>(send), [this, &req, &send](const app::Token& token) {

            json::object response_json;

            boost::json::object players_json;
            boost::json::object lost_objects_json;

            std::vector<std::shared_ptr<model::GameSession>> sessions = application_.GetGame().GetAllSession();
            for (std::shared_ptr<model::GameSession>& session : sessions) {
                net::dispatch(*session->GetSessionStrand(), [self = shared_from_this(), req = std::move(req), &players_json, &lost_objects_json, &session, send = std::forward<Send>(send)]()  {

                    for (const std::shared_ptr<model::Dog>& dog : session->GetDogs()) {
                        boost::json::object dog_json;
                        dog_json["pos"] = {dog->GetCoordinate().x, dog->GetCoordinate().y};
                        dog_json["speed"] = {dog->GetSpeed().first, dog->GetSpeed().second};

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
                            default:
                                dog_json["dir"] = "Unknown";
                                break;
                        }

                        boost::json::array bag_json;
                        for (const auto& item : dog->GetBag()) {
                            boost::json::object item_json;
                            item_json["id"] = item->GetId();
                            item_json["type"] = item->GetType();
                            bag_json.push_back(item_json);
                        }
                        dog_json["bag"] = bag_json;
                        dog_json["score"] = dog->GetScore();

                        players_json[std::to_string(dog->GetId())] = dog_json;
                    }
                    for (const std::shared_ptr<model::LostObject>& lost_object: session->GetLostObjects()) {
                        boost::json::object lost_object_json;
                        lost_object_json["type"] = lost_object->GetType();
                        lost_object_json["pos"] = {lost_object->GetCoordinate().x, lost_object->GetCoordinate().y};

                        lost_objects_json[std::to_string(lost_object->GetId())] = lost_object_json;
                    }
                });
            }

            response_json["players"] = players_json;
            response_json["lostObjects"] = lost_objects_json;
            SendJsonResponse(response_json, std::forward<Send>(send));
        });
    }

    template <typename Send>
    void HandleSetPlayersTick(const http::request<http::string_body>& req, Send&& send) {

        if (req.method() != http::verb::post) {
            const std::string allowedMethods = "POST";
            SendErrorResponse("invalidMethod", "Only POST method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        if (req[http::field::content_type] != "application/json") {
            SendErrorResponse("invalidArgument", "Invalid Content-Type", http::status::bad_request, std::forward<Send>(send));
            return;
        }

        try {

            json::value parsedJson = json::parse(req.body());
            json::object obj = parsedJson.as_object();

            if (!obj.contains("timeDelta") || !obj["timeDelta"].is_int64()) {
                SendErrorResponse("invalidArgument", "Failed to parse action", http::status::bad_request, std::forward<Send>(send));
                return;
            }

            int time_delta = obj["timeDelta"].as_int64();
            std::chrono::milliseconds delta(time_delta);

            std::vector<std::shared_ptr<model::GameSession>> sessions = application_.GetGame().GetAllSession();
            for (std::shared_ptr<model::GameSession>& session : sessions) {
                net::dispatch(*session->GetSessionStrand(), [self = shared_from_this(), &delta, &session, req = std::move(req), send = std::forward<Send>(send)]() mutable {
                    session->UpdateSessionByTime(delta);
                });
            }
            application_.SaveGameByTime(delta);
        } catch (const std::exception& e) {
            SendErrorResponse("invalidArgument", "Failed to parse action", http::status::bad_request, std::forward<Send>(send));
        }
        json::object response_json = {};
        SendJsonResponse(response_json, std::forward<Send>(send));
    }

    std::map<std::string, std::string> ParseQuery(const std::string& query);

    template <typename Send>
    void HandleGetTableRecords(const http::request<http::string_body>& req, Send&& send) {
        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            const std::string allowedMethods = "GET, HEAD";
            SendErrorResponse("invalidMethod", "Only Get method is expected", http::status::method_not_allowed, std::forward<Send>(send), allowedMethods);
            return;
        }

        std::string target = std::string(req.target());
        std::string::size_type query_pos = target.find('?');
        std::map<std::string, std::string> params;

        if (query_pos != std::string::npos) {
            std::string query = target.substr(query_pos + 1);
            params = ParseQuery(query);
        }

        size_t  start{0};
        size_t  maxItems{constants::MAX_TABLE_ITEMS};

        if (params.find("start") != params.end()) {
            try {
                start = std::stoul(params["start"]);
            } catch (const std::exception&) {
                SendErrorResponse("invalidArgument start", "Invalid Content-Type", http::status::bad_request, std::forward<Send>(send));
                return;
            }
        }

        if (params.find("maxItems") != params.end()) {
            try {
                maxItems = std::stoul(params["maxItems"]);
                if (maxItems > constants::MAX_TABLE_ITEMS) {
                    SendErrorResponse("invalidArgument maxItems", "Invalid Content-Type items > maxSize", http::status::bad_request, std::forward<Send>(send));
                    return;
                }
            } catch (const std::exception&) {
                SendErrorResponse("invalidArgument maxItems", "Invalid Content-Type", http::status::bad_request, std::forward<Send>(send));
                return;
            }
        }

        auto retired_players = application_.GetTableRecords(start, maxItems);

        boost::json::array response_json;
        for (const auto& player : retired_players) {
            boost::json::object player_json;
            player_json["name"] = player.GetName();
            player_json["score"] = player.GetScore();
            player_json["playTime"] = player.GetPlayTime() / 1000.0;
            response_json.push_back(player_json);
        }
        SendJsonResponse(response_json, std::forward<Send>(send));
    }


    template <typename Fn, typename Send>
    void ExecuteAuthorized(const http::request<http::string_body>& req, Send&& send, Fn&& action) {

        auto authHeader = req.find(http::field::authorization);

        if (authHeader == req.end()) {
            SendErrorResponse("invalidToken", "Authorization header is required", http::status::unauthorized, std::forward<Send>(send));
            return;
        }

        std::string tokenStr = authHeader->value().to_string();
        const std::string bearerPrefix = "Bearer ";
        if (tokenStr.rfind(bearerPrefix, 0) == 0) {
            tokenStr = tokenStr.substr(bearerPrefix.size());
        } else {
            SendErrorResponse("invalidToken", "Invalid token format", http::status::unauthorized, std::forward<Send>(send));
            return;
        }

        if (tokenStr.size() != 32) {
            SendErrorResponse("invalidToken", "Invalid token format", http::status::unauthorized, std::forward<Send>(send));
            return;
        }

        app::Token token(tokenStr);
        auto player = application_.GetPlayerTokens().FindPlayerByToken(token);
        if (!player) {
            SendErrorResponse("unknownToken", "Player token has not been found", http::status::unauthorized, std::forward<Send>(send));
            return;
        }

        action(token);
    }

    json::object CreateMapJson(const model::Map& map);
    json::object SerializeRoad(const model::Road& road);
    json::object SerializeBuilding(const model::Building& building);
    json::object SerializeOffice(const model::Office& office);
    json::object SerializeLootType(const model::LootType &loot_type);
};

} //namespace http_handler
