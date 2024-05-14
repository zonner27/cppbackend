#include "json_loader.h"


namespace json_loader {

namespace json = boost::json;
using namespace std::literals;

model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    model::Game game;

    std::ifstream file(json_path);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл." << std::endl;
    }

    std::string jsonDataStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    json::value jsonData;
    try {
        jsonData = json::parse(jsonDataStr);
    } catch (std::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
    }

    const auto& mapsArray = jsonData.as_object()["maps"].as_array();
    for (const auto& mapData : mapsArray) {
        std::string mapId = mapData.as_object().at("id").as_string().c_str();
        std::string mapName = mapData.as_object().at("name").as_string().c_str();
        model::Map map(model::Map::Id(mapId), mapName);

        for (const auto& roadData : mapData.as_object().at("roads").as_array()) {
            int x0 = roadData.as_object().at("x0").as_int64();
            int y0 = roadData.as_object().at("y0").as_int64();
            int x1 = roadData.as_object().contains("x1") ? roadData.as_object().at("x1").as_int64() : x0;
            int y1 = roadData.as_object().contains("y1") ? roadData.as_object().at("y1").as_int64() : y0;

            model::Point start{x0, y0};
            if (x0 == x1) {
                map.AddRoad(model::Road(model::Road::VERTICAL, start, y1));
            } else if (y0 == y1) {
                map.AddRoad(model::Road(model::Road::HORIZONTAL, start, x1));
            } else {
                std::cerr << "Invalid road data in map: " << mapId << std::endl;
            }
        }

        for (const auto& buildingData : mapData.as_object().at("buildings").as_array()) {
            int x = buildingData.as_object().at("x").as_int64();
            int y = buildingData.as_object().at("y").as_int64();
            int w = buildingData.as_object().at("w").as_int64();
            int h = buildingData.as_object().at("h").as_int64();

            map.AddBuilding(model::Building({{x, y}, {w, h}}));
        }

        for (const auto& officeData : mapData.as_object().at("offices").as_array()) {
            std::string officeIdString = officeData.as_object().at("id").as_string().c_str();
            model::Office::Id officeId(officeIdString);
            int x = officeData.as_object().at("x").as_int64();
            int y = officeData.as_object().at("y").as_int64();
            int offsetX = officeData.as_object().at("offsetX").as_int64();
            int offsetY = officeData.as_object().at("offsetY").as_int64();

            map.AddOffice(model::Office(officeId, {x, y}, {offsetX, offsetY}));
        }

        game.AddMap(map);
    }

    return game;
}

}
