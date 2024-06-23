#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <memory>
#include <map>
#include <optional>
#include <cmath>

#include "../tagged.h"
#include "../constants.h"

#include <iostream>

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
    bool operator<(const Point& other) const {
        return std::tie(x, y) < std::tie(other.x, other.y);
    }
    bool operator>(const Point& other) const {
        return other < *this;
    }
};


struct Coordinates {
    double x;
    double y;
};


struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

private:
    Point start_;
    Point end_;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using RoadMap = std::map<int, Roads>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    void AddRoad(const Road& road) {
        roads_.push_back(road);
        if (start_roads_point_.x == -1) {
            start_roads_point_ = road.GetStart();
            //std::cout << " start_road_point x " << start_roads_point_.x << " y =  " << start_roads_point_.y << std::endl;
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

    void PrintHorRoads() const {
        std::cout << "Horizontal roads:\n";
        for (const auto& pair : hor_roads_) {
            int y = pair.first;
            const std::vector<Road>& roads = pair.second;
            for (const Road& road : roads) {
                std::cout << "  From (" << road.GetStart().x << ", " << y << ") to (" << road.GetEnd().x << ", " << y << ")\n";
            }
        }
    }

    void PrintVerRoads() const {
        std::cout << "Vertical roads:\n";
        for (const auto& pair : ver_roads_) {
            int x = pair.first;
            const std::vector<Road>& roads = pair.second;
            for (const Road& road : roads) {
                std::cout << "  From (" << x << ", " << road.GetStart().y << ") to (" << x << ", " << road.GetEnd().y << ")\n";
            }
        }
    }

    const Road* GetHorRoad(Coord x, Coord y) const {
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

    const Road* GetVerRoad(Coord x, Coord y) const {
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

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

    void SetDogSpeed(double dogSpeed) {
        dogSpeed_ = dogSpeed;
    }

    const double GetDogSpeed() const noexcept{
        return dogSpeed_;
    }

    const Point& GetStartPointRoadMap() const noexcept{
        return start_roads_point_;
    }


private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    //RoadMatrix road_matrix_;
    Point start_roads_point_{-1, -1};
    RoadMap hor_roads_;
    RoadMap ver_roads_;
    //std::unordered_map<Coord, bool> roads_map_;
    Buildings buildings_;
    double dogSpeed_;


    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

}  // namespace model
