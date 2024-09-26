#pragma once

#include <filesystem>
#include <cassert>
#include <string>
#include "constants.h"

namespace fs = std::filesystem;
using namespace std::literals;

namespace files_path {

    bool IsSubPath(fs::path path, fs::path base);
    std::string UrlDecode(const std::string& url);
    std::string MimeDecode(fs::path& path);

}
