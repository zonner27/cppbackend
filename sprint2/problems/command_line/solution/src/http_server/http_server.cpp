#include "http_server.h"


namespace http_server {

void SessionBase::ReportError(beast::error_code ec, std::string_view what) {
    json::value custom_data = json::object{
            {"code"s, ec.value()},
            {"text"s, ec.message()},
            {"where"s, what}
    };
    BOOST_LOG_TRIVIAL(error) << logging::add_value(additional_data, custom_data) << "error"sv;
}

void SessionBase::Run() {
    // Вызываем метод Read, используя executor объекта stream_.
    // Таким образом вся работа со stream_ будет выполняться, используя его executor
    net::dispatch(stream_.get_executor(),
                  beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
}

}  // namespace http_server
