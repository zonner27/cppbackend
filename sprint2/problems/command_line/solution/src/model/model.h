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
#include <random>

#include "../tagged.h"
#include "../constants.h"

#include <iostream>

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
    Point& operator=(const Point& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }
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

    static int GetRandomNumber(int min, int max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }

    Point GetRandomPointOnRoad(const Road& road) const {
        Point start = road.GetStart();
        Point end = road.GetEnd();
        //std::cout << " road start x = " << start.x << " y = " << start.y << std::endl;
        //std::cout << " road end x = " << end.x << " y = " << end.y << std::endl;

        if (road.IsHorizontal()) {
            Dimension random_x;
            if (start.x <=  end.x) {
                random_x = GetRandomNumber(start.x, end.x);
            } else {
                random_x = GetRandomNumber(end.x, start.x);
            }
            //std::cout << " random_x= " << random_x << " start.y " << start.y << std::endl;
            return {random_x, start.y};
        } else if (road.IsVertical()) {
            Dimension random_y;
            if (start.x <=  end.x) {
                random_y = GetRandomNumber(start.y, end.y);
            } else {
                random_y = GetRandomNumber(end.y, start.y);
            }
            //std::cout << "start.x = " << start.x << " random_y = " << random_y << std::endl;
            return {start.x, random_y};
        }
        return start;
    }

    Point GetRandomPointRoadMap() const {
        if (roads_.empty()) {
            throw std::runtime_error("Road list is empty");
        }
        size_t random_index = GetRandomNumber(0, roads_.size() - 1);
        //std::cout << "random index = " << random_index << std::endl;
        const Road& random_road = roads_[random_index];
        return GetRandomPointOnRoad(random_road);
    }

    const Point& GetStartPointRoadMap() const noexcept{
        return start_roads_point_;
    }


private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Point start_roads_point_{-1, -1};
    RoadMap hor_roads_;
    RoadMap ver_roads_;
    Buildings buildings_;
    double dogSpeed_;


    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

}  // namespace model
