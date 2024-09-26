#pragma once

#include <string>
#include <unordered_map>
#include <iomanip>
#include <sstream>

//#define ENABLE_SYNC_WRITE

namespace constants {

    const std::string X = "x";
    const std::string Y = "y";
    const std::string W = "w";
    const std::string H = "h";
    const std::string X0 = "x0";
    const std::string Y0 = "y0";
    const std::string X1 = "x1";
    const std::string Y1 = "y1";
    const std::string OFFSET_X = "offsetX";
    const std::string OFFSET_Y = "offsetY";
    const std::string ID = "id";
    const std::string MAPS = "maps";
    const std::string NAME = "name";
    const std::string ROADS = "roads";
    const std::string BUILDINGS = "buildings";
    const std::string OFFICES = "offices";
    const std::string CODE = "code";
    const std::string MESSAGE = "message";

    const std::string LOOT_TYPES =  "lootTypes";
    const std::string FILE = "file";
    const std::string TYPE = "type";
    const std::string ROTATION = "rotation";
    const std::string COLOR = "color";
    const std::string SCALE = "scale";
    const std::string VALUE = "value";

    const std::unordered_map<std::string, std::string> MIME_TYPES = {
        {".htm", "text/html"}, {".html", "text/html"},
        {".css", "text/css"}, {".txt", "text/plain"},
        {".js", "text/javascript"}, {".json", "application/json"},
        {".xml", "application/xml"}, {".png", "image/png"},
        {".jpg", "image/jpeg"}, {".jpe", "image/jpeg"},
        {".jpeg", "image/jpeg"}, {".gif", "image/gif"},
        {".bmp", "image/bmp"}, {".ico", "image/vnd.microsoft.icon"},
        {".tiff", "image/tiff"}, {".tif", "image/tiff"},
        {".svg", "image/svg+xml"}, {".svgz", "image/svg+xml"},
        {".mp3", "audio/mpeg"}
    };

    const uint32_t MAXPLAYERSINMAP = 20;
    const double MAXDISTANCEFROMCENTER = 0.4;

    enum class Direction {
        NORTH,
        SOUTH,
        WEST,
        EAST,
        STOP
    };

    const double WIDTH_ITEM = 0;
    const double WIDTH_PLAYER = 0.6;
    const double WIDTH_BASE = 0.5;

    const uint32_t MAX_TABLE_ITEMS = 100;

}  // namespace constants
