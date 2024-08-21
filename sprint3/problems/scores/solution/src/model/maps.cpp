#include "maps.h"

namespace model {

const Map::Id &Map::GetId() const noexcept {
    return id_;
}

const std::string &Map::GetName() const noexcept {
    return name_;
}

const Map::Buildings &Map::GetBuildings() const noexcept {
    return buildings_;
}

const Map::Roads &Map::GetRoads() const noexcept {
    return roads_;
}

const Map::Offices &Map::GetOffices() const noexcept {
    return offices_;
}

const Map::LootTypes &Map::GetLootTypes() const noexcept {
    return loot_types_;
}

void Map::AddRoad(const Road &road) {
    roads_.push_back(road);
    if (start_roads_point_.x == -1) {
        start_roads_point_ = road.GetStart();
    }
    if (road.IsHorizontal()) {
        if (road.GetStart().x <= road.GetEnd().x) {
            hor_roads_[road.GetStart().y].push_back(road);
        } else {
            hor_roads_[road.GetStart().y].push_back(model::Road(model::Road::HORIZONTAL, road.GetEnd(), road.GetStart().x));
        }
    } else {
        if (road.GetStart().y <= road.GetEnd().y) {
            ver_roads_[road.GetStart().x].push_back(road);
        } else {
            ver_roads_[road.GetStart().x].push_back(model::Road(model::Road::HORIZONTAL, road.GetEnd(), road.GetStart().y));
        }
    }
}

const Road *Map::GetHorRoad(Coord x, Coord y) const {
    auto it = hor_roads_.find(y);
    if (it != hor_roads_.end()) {
        const std::vector<Road>& roads = it->second;
        for (const Road& road : roads) {
            if (x >= road.GetStart().x && x <= road.GetEnd().x) {
                return &road;
            }
        }
    }
    return nullptr;
}

const Road *Map::GetVerRoad(Coord x, Coord y) const {
    auto it = ver_roads_.find(x);
    if (it != ver_roads_.end()) {
        const std::vector<Road>& roads = it->second;
        for (const Road& road : roads) {
            if (road.GetStart().y <= y && y <= road.GetEnd().y) {
                return &road;
            }
        }
    }
    return nullptr;
}

void Map::AddBuilding(const Building &building) {
    buildings_.emplace_back(building);
}

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

void Map::AddLootType(const LootType &loot_type) {
    loot_types_.emplace_back(loot_type);
}

void Map::SetDogSpeed(double dogSpeed) {
    dogSpeed_ = dogSpeed;
}

const double Map::GetDogSpeed() const noexcept{
    return dogSpeed_;
}

void Map::SetBagCapaccity(int bagCapacity) {
    bagCapacity_ = bagCapacity;
}

const int Map::GetBagCapacity() const noexcept {
    return bagCapacity_;
}

int Map::GetRandomNumber(int min, int max) {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<int> distr(min, max);
    return distr(eng);
}

Point Map::GetRandomPointOnRoad(const Road &road) const {
    Point start = road.GetStart();
    Point end = road.GetEnd();

    if (road.IsHorizontal()) {
        Dimension random_x;
        if (start.x <=  end.x) {
            random_x = GetRandomNumber(start.x, end.x);
        } else {
            random_x = GetRandomNumber(end.x, start.x);
        }
        return {random_x, start.y};
    } else if (road.IsVertical()) {
        Dimension random_y;
        if (start.y <=  end.y) {
            random_y = GetRandomNumber(start.y, end.y);
        } else {
            random_y = GetRandomNumber(end.y, start.y);
        }
        return {start.x, random_y};
    }
    return start;
}

Point Map::GetRandomPointRoadMap() const {
    if (roads_.empty()) {
        throw std::runtime_error("Road list is empty");
    }
    size_t random_index = GetRandomNumber(0, roads_.size() - 1);
    const Road& random_road = roads_[random_index];
    return GetRandomPointOnRoad(random_road);
}

const Point &Map::GetStartPointRoadMap() const noexcept{
    return start_roads_point_;
}

uint32_t Map::GetScoreByLootType(size_t id) {
    return loot_types_[id].value;
}

} // namespace model
