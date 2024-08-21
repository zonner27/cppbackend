#pragma once

#include "request_handler.h"

namespace http_handler {

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
                {"ip"s, client_ip},
                {"URI"s, req.target().to_string()},
                {"method"s, req.method_string().to_string()}
        };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "request received"sv;
    }

    template <typename Body>
    void LogResponse(const http::response<Body>& resp, int64_t response_time, const std::string& client_ip) {

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


} //namespace http_handler
