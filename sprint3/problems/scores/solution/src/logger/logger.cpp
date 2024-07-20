#include "logger.h"

void JsonFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {

    json::object data_strm;

    if (auto ts = rec[timestamp]; ts) {
        data_strm["timestamp"s] = boost::posix_time::to_iso_extended_string(ts.get());
    }
    if (auto data = rec[additional_data]; data) {
        data_strm["data"s] = data.get();
    }
    if (auto msg = rec[expressions::smessage]; msg) {
        data_strm["message"s] = msg.get();
    }

    strm << json::serialize(data_strm);
}

void LoggerInit()
{
    logging::add_common_attributes();

    logging::add_console_log(
        std::cout,
        keywords::format = &JsonFormatter,
        keywords::auto_flush = true
    );
}

