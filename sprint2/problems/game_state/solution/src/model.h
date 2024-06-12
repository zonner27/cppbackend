#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <memory>

#include "tagged.h"
#include "constants.h"

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
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
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

//Алгоритм добавления собаки в игровой сеанс будет выглядеть так.
//Найти игровой сеанс, соответствующий карте, на которой хочет играть клиент.
//Внутри игрового сеанса добавить нового пса с указанным именем и сгенерированным id.

struct Coordinates {
    double x;
    double y;
};

enum class Direction {
    NORTH,
    SOUTH,
    WEST,
    EAST
};

class Dog{
public:
    Dog(std::string& name) : name_{name}, dog_id{nextId++} {
        coordinates_.x = 0;
        coordinates_.y = 0;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const uint32_t GetId() const noexcept {
        return dog_id;
    }

    const Coordinates& GetCoordinate() const noexcept {
        return coordinates_;
    }

    const std::pair<double, double>& GetSpeed() const noexcept {
        return speed_;
    }

    const Direction& GetDirection() const noexcept {
        return direction_;
    }

private:
    std::string name_;
    static uint32_t nextId;
    std::uint32_t dog_id = 0;
    Coordinates coordinates_;
    std::pair<double, double> speed_ = {0.0, 0.0};
    Direction direction_ = Direction::NORTH;
};


class GameSession {
public:
//    GameSession(Dog& dog, const Map* map) : map_{map} {
//        dogs_.insert(dog);
//    };
    using Id = util::Tagged<std::string, Map>;

    GameSession(const Map* map) : map_{map} {}

    void AddDog(Dog& dog) {
        dogs_.push_back(dog);
    }

    const std::string& GetMapName() const noexcept {
        return map_->GetName();
    }

    const Id& GetId() const noexcept {
        return map_->GetId();
    }

    const size_t GetDogsCount() const noexcept {
        return dogs_.size();
    }

    std::vector<Dog>& GetDogs() noexcept {
        return dogs_;
    }


private:
    std::vector<Dog> dogs_;
    const Map* map_;
};



class Game {
public:
    using Maps = std::vector<Map>;

    void AddMap(Map map);

    void AddSession(std::shared_ptr<GameSession> session) {
        sessions_.push_back(session);
    }

    std::vector<std::shared_ptr<GameSession>>& GetAllSession() {
        return sessions_;
    }

    std::shared_ptr<GameSession> FindValidSession(const Map* map) {
        for (const auto& session : sessions_) {
            if (session->GetMapName() == map->GetName() && session->GetDogsCount() < constants::MAXPLAYERSINMAP)
                return session;
        }
        auto newSession = std::make_shared<GameSession>(map);
        AddSession(newSession);
        return newSession;
    }

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;

    //std::vector<GameSession> sessions;
    std::vector<std::shared_ptr<GameSession>> sessions_;
};



}  // namespace model


//    Dog(const Dog& other) : name_{other.name_}, dog_id{other.dog_id} {}

//    Dog& operator=(const Dog& other) {
//        if (this != &other) {
//            name_ = other.name_;
//            dog_id = other.dog_id;
//        }
//        return *this;
//    }

//    Dog(Dog&& other) noexcept : name_{std::move(other.name_)}, dog_id{other.dog_id} {}

//    Dog& operator=(Dog&& other) noexcept {
//        if (this != &other) {
//            name_ = std::move(other.name_);
//            dog_id = other.dog_id;
//        }
//        return *this;
//    }

//    bool operator==(const Dog& other) const {
//       return name_ == other.name_;
//    }

//    bool operator<(const Dog& other) const {
//       return name_ < other.name_;
//    }

//    bool operator>(const Dog& other) const {
//       return other < *this;
//    }
