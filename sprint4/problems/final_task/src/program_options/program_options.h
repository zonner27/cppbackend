#pragma once

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>
#include <boost/filesystem.hpp>

using namespace std::literals;

struct Args {
    uint32_t tick_period{0};
    std::string config_file;
    std::string www_root;
    bool randomize_spawn_points{false};
    boost::filesystem::path base_path;
    std::string state_file{};
    uint32_t save_state_period{0};
};

[[nodiscard]] std::optional<Args>  ParseCommandLine(int argc, const char* const argv[]);

