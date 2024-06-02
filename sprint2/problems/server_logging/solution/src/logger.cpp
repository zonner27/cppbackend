#include "logger.h"


void json_formatter(logging::record_view const& rec, logging::formatting_ostream& strm) {

    json::object root;

    auto ts = rec[timestamp];
    if (ts)
    {
        std::stringstream ss;
        ss << boost::posix_time::to_iso_extended_string(*ts);
        root["timestamp"] = ss.str();
    }

    auto data = rec[additional_data];
    if (data)
    {
        root["data"] = *data;
    }

    auto msg = rec[expr::smessage];
    if (msg)
    {
        root["message"] = *msg;
    }

    strm << json::serialize(root);
}

void logger_init()
{
    logging::add_common_attributes();

    logging::add_console_log(
        std::clog,
        keywords::format = &json_formatter
    );
}
