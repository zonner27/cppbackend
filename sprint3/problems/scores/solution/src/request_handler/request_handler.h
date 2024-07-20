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
    explicit BaseRequestHandler(app::Application& application, fs::path static_path)
        : application_{application}, static_path_{static_path} {}

    BaseRequestHandler(const BaseRequestHandler&) = delete;
    BaseRequestHandler& operator=(const BaseRequestHandler&) = delete;

protected:    
    fs::path static_path_;
    app::Application& application_;


    template <typename Send>
    void SendJsonResponse(const json::value& jsonResponse, Send&& send) {
        StringResponse response;
        response.result(http::status::ok);
        response.set(http::field::content_type, "application/json");    
        std::string body = boost::json::serialize(jsonResponse);
        response.body() = body;
        response.content_length(body.size());
        response.set(http::field::cache_control, "no-cache");

#ifdef ENABLE_SYNC_WRITE
        SyncWriteOStreamAdapter adapter{std::cout};
        http::write(adapter, response);
#endif

        send(std::move(response));
    }

    template <typename Send>
    void SendErrorResponse(const std::string& code, const std::string& message, http::status status, Send&& send,
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

#ifdef ENABLE_SYNC_WRITE
        SyncWriteOStreamAdapter adapter{std::cout};
        http::write(adapter, response);
#endif

        send(std::move(response));
    }

    template <typename Send>
    void SendTextResponse(const std::string& text, http::status status, Send&& send) {
        http::response<http::string_body> response;
        response.result(status);
        response.set(http::field::content_type, "text/plain");
        response.body() = text;
        response.prepare_payload();
        send(std::move(response));
    }

    template <typename Send>
    void SendFileResponse(fs::path& file_path, Send&& send) {
        std::string mime_types = files_path::MimeDecode(file_path);
        http::file_body::value_type file;

        if (sys::error_code ec; file.open(file_path.string().c_str(), beast::file_mode::read, ec), ec) {
            SendTextResponse("Failed to open file: " + ec.message(), http::status::internal_server_error, std::forward<Send>(send));
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

} //namespace http_handler
