#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/date_time.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/json.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string_view>

using namespace std::literals;
namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace json = boost::json;
namespace expressions = boost::log::expressions;

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::log::attributes::local_clock::value_type)

void JsonFormatter(logging::record_view const& rec, logging::formatting_ostream& strm);
void LoggerInit();

