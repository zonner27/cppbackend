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
        throw std::runtime_error("Failed to open file: " + json_path.string());
    }

    std::string jsonDataStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    json::value jsonData;
    try {
        jsonData = json::parse(jsonDataStr);
    } catch (std::exception& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }

    if (jsonData.as_object().contains("defaultDogSpeed")) {
       game.SetDefaultDogSpeed(jsonData.as_object().at("defaultDogSpeed").as_double());
    }

    if (jsonData.as_object().contains("defaultBagCapacity")) {
       game.SetDefaultBagCapacity(jsonData.as_object().at("defaultBagCapacity").as_int64());
    }

    if (jsonData.as_object().contains("lootGeneratorConfig")) {
        const auto& loot_gen_config_json = jsonData.at("lootGeneratorConfig").as_object();

        model::LootGeneratorConfig loot_gen_config;
        loot_gen_config.period = loot_gen_config_json.at("period").as_double();
        loot_gen_config.probability = loot_gen_config_json.at("probability").as_double();

        game.SetLootGeneratorConfig(loot_gen_config);
    }

    if (jsonData.as_object().contains("dogRetirementTime")) {
        double time_s = jsonData.as_object().at("dogRetirementTime").as_double();
        uint32_t time_ms = static_cast<uint32_t>(time_s * 1000);
        model::Dog::SetRetirementTime(time_ms);
    }

    const auto& mapsArray = jsonData.as_object()[constants::MAPS].as_array();
    for (const auto& mapData : mapsArray) {
        std::string mapId = mapData.as_object().at(constants::ID).as_string().c_str();
        std::string mapName = mapData.as_object().at(constants::NAME).as_string().c_str();
        model::Map map(model::Map::Id(mapId), mapName);

        double dogSpeed = game.GetDefaultDogSpeed();
        if (mapData.as_object().contains("dogSpeed")) {
            dogSpeed = mapData.as_object().at("dogSpeed").as_double();
        }
        map.SetDogSpeed(dogSpeed);

        int bagCapacity = game.GetDefaultBagCapacity();
        if (mapData.as_object().contains("bagCapacity")) {
            bagCapacity = mapData.as_object().at("bagCapacity").as_int64();
        }
        map.SetBagCapaccity(bagCapacity);

        try {
            ParseRoads(mapData, map);
            ParseBuildings(mapData, map);
            ParseOffices(mapData, map);
            ParseLootTypes(mapData, map);
        } catch (const std::exception& e) {
            throw std::runtime_error("Error while parsing map data in map: " + mapId + ": " + std::string(e.what()));
        }
        game.AddMap(map);
    }

    return game;
}

void ParseRoads(const json::value& mapData, model::Map& map) {
    const auto& roadsArray = mapData.as_object().at(constants::ROADS).as_array();
    for (const auto& roadData : roadsArray) {
        int x0 = roadData.as_object().at(constants::X0).as_int64();
        int y0 = roadData.as_object().at(constants::Y0).as_int64();
        int x1 = roadData.as_object().contains(constants::X1) ? roadData.as_object().at(constants::X1).as_int64() : x0;
        int y1 = roadData.as_object().contains(constants::Y1) ? roadData.as_object().at(constants::Y1).as_int64() : y0;

        model::Point start{x0, y0};
        if (x0 == x1) {
            map.AddRoad(model::Road(model::Road::VERTICAL, start, y1));
        } else if (y0 == y1) {
            map.AddRoad(model::Road(model::Road::HORIZONTAL, start, x1));
        } else {
            assert(false && "Invalid road data in map");
            throw std::runtime_error("Invalid road data in map");
        }
    }
}

void ParseBuildings(const json::value& mapData, model::Map& map) {
    const auto& buildingsArray = mapData.as_object().at(constants::BUILDINGS).as_array();
    for (const auto& buildingData : buildingsArray) {
        int x = buildingData.as_object().at(constants::X).as_int64();
        int y = buildingData.as_object().at(constants::Y).as_int64();
        int w = buildingData.as_object().at(constants::W).as_int64();
        int h = buildingData.as_object().at(constants::H).as_int64();

        map.AddBuilding(model::Building({{x, y}, {w, h}}));
    }
}

void ParseOffices(const json::value& mapData, model::Map& map) {
    const auto& officesArray = mapData.as_object().at(constants::OFFICES).as_array();
    for (const auto& officeData : officesArray) {
        std::string officeIdString = officeData.as_object().at(constants::ID).as_string().c_str();
        model::Office::Id officeId(officeIdString);
        int x = officeData.as_object().at(constants::X).as_int64();
        int y = officeData.as_object().at(constants::Y).as_int64();
        int offsetX = officeData.as_object().at(constants::OFFSET_X).as_int64();
        int offsetY = officeData.as_object().at(constants::OFFSET_Y).as_int64();

        map.AddOffice(model::Office(officeId, {x, y}, {offsetX, offsetY}));
    }
}

void ParseLootTypes(const boost::json::value &mapData, model::Map &map) {

    const auto& lootTypesArray = mapData.as_object().at(constants::LOOT_TYPES).as_array();

    for (const auto& lootTypeData : lootTypesArray) {
        model::LootType lootType;

        if (lootTypeData.as_object().contains(constants::NAME)) {
            lootType.name = lootTypeData.as_object().at(constants::NAME).as_string().c_str();
        }
        if (lootTypeData.as_object().contains(constants::FILE)) {
            lootType.file = lootTypeData.as_object().at(constants::FILE).as_string().c_str();
        }
        if (lootTypeData.as_object().contains(constants::TYPE)) {
            lootType.type = lootTypeData.as_object().at(constants::TYPE).as_string().c_str();
        }
        if (lootTypeData.as_object().contains(constants::ROTATION)) {
            lootType.rotation = lootTypeData.as_object().at(constants::ROTATION).as_int64();
        }
        if (lootTypeData.as_object().contains(constants::COLOR)) {
            lootType.color = lootTypeData.as_object().at(constants::COLOR).as_string().c_str();
        }
        if (lootTypeData.as_object().contains(constants::SCALE)) {
            lootType.scale = lootTypeData.as_object().at(constants::SCALE).as_double();
        }
        if (lootTypeData.as_object().contains(constants::VALUE)) {
            lootType.value = lootTypeData.as_object().at(constants::VALUE).as_int64();
        }

        map.AddLootType(lootType);
    }
}

}
