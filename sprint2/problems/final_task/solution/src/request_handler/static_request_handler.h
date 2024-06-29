#pragma once
#include "request_handler.h"


namespace http_handler {

class StaticFileRequestHandler : public BaseRequestHandler {
public:
    using BaseRequestHandler::BaseRequestHandler;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (req.method() == http::verb::get || req.method() == http::verb::head) {
            HandleStaticFileRequest(std::move(req), std::forward<Send>(send));
        }
    }

private:
    template <typename Body, typename Allocator, typename Send>
    void HandleStaticFileRequest(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        std::string path_str = req.target().to_string();

        if (!path_str.empty() && path_str[0] == '/') {
            path_str = path_str.substr(1);
            path_str = files_path::UrlDecode(path_str);
        }

        if (path_str.empty()) {
            path_str = "index.html";
        }

        fs::path file_path = static_path_ / fs::path(path_str);

        if (!files_path::IsSubPath(file_path, static_path_)) {
            SendTextResponse("Invalid request: path is outside of the static directory\n", http::status::bad_request, std::forward<Send>(send));
            return;
        }

        if (fs::exists(file_path) && fs::is_regular_file(file_path)) {
            SendFileResponse(file_path, std::forward<Send>(send));
        } else {
            SendTextResponse("Invalid request: File does not exist\n", http::status::not_found, std::forward<Send>(send));
        }
    }
};


} //namespace http_handler
