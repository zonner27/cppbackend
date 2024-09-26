#pragma once
#include <cstdint>
#include <string>

namespace db_app {

struct DBSettings {
    uint32_t number_connections{1};
    std::string db_url{};
};

}   //namespace db_app

