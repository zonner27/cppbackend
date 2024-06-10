#pragma once
#include "http_server.h"
#include "model.h"
#include "constants.h"
#include "files.h"
#include "streamadapter.h"
#include "logger.h"

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
    explicit BaseRequestHandler(model::Game& game, fs::path static_path)
        : game_{game}, static_path_{static_path} {}

    BaseRequestHandler(const BaseRequestHandler&) = delete;
    BaseRequestHandler& operator=(const BaseRequestHandler&) = delete;

protected:
    model::Game& game_;
    fs::path static_path_;

    template <typename Send>
    void sendJsonResponse(const json::value& jsonResponse, Send&& send) {
        StringResponse response;
        response.result(http::status::ok);
        response.set(http::field::content_type, "application/json");
        response.body() = boost::json::serialize(jsonResponse);
        send(std::move(response));
    }

    template <typename Send>
    void sendErrorResponse(const std::string& code, const std::string& message, http::status status, Send&& send) {
        boost::json::object errorObject;
        errorObject["code"] = code;
        errorObject["message"] = message;

        StringResponse response;
        response.result(status);
        response.set(http::field::content_type, "application/json");
        response.body() = boost::json::serialize(errorObject);
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

class ApiRequestHandler : public BaseRequestHandler {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    explicit ApiRequestHandler(model::Game& game, fs::path static_path,  Strand& api_strand) //net::io_context& ioc,
            : BaseRequestHandler(game, static_path), api_strand_{api_strand} {}

    using BaseRequestHandler::BaseRequestHandler;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        net::dispatch(api_strand_, [this, req = std::move(req), send = std::forward<Send>(send)]() mutable {
            if (req.target() == "/api/v1/maps" && req.method() == http::verb::get) {
                handleGetMapsRequest(std::forward<Send>(send));
            } else if (req.method() == http::verb::get && req.target().starts_with("/api/v1/maps/")) {
                handleGetMapByIdRequest(req.target().to_string().substr(13), std::forward<Send>(send));
            } else {
                sendErrorResponse("badRequest", "Bad request", http::status::bad_request, std::forward<Send>(send));
            }
        });
    }

private:
    Strand& api_strand_;

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
