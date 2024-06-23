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

//Алгоритм добавления собаки в игровой сеанс будет выглядеть так.
//Найти игровой сеанс, соответствующий карте, на которой хочет играть клиент.
//Внутри игрового сеанса добавить нового пса с указанным именем и сгенерированным id.




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

    void SetCoordinate(const Coordinates& coordinates) noexcept {
        coordinates_ = coordinates;
    }

    void SetCoordinateByPoint(const Point& point) noexcept {
        coordinates_.x = static_cast<double>(point.x);
        coordinates_.y = static_cast<double>(point.y);
    }

    Coordinates GetCoordinateByTime(int time_delta) noexcept {
        Coordinates finish;
        std::cout << std::fixed << std::setprecision(4);
        finish.x = coordinates_.x + speed_.first * static_cast<double>(time_delta) / 1000;
        finish.y = coordinates_.y + speed_.second * static_cast<double>(time_delta) / 1000;
//        std::cout << "speed_.first = " << speed_.first << " speed_.second = " << speed_.second << std::endl;
//        std::cout << "coordinates_.x = " << coordinates_.x<< " coordinates_.y = " << coordinates_.y << std::endl;
//        std::cout << "time = " << static_cast<double>(time_delta / 1000) << std::endl;
//        std::cout << "finish coordinate x = " << finish.x << " finish coordinate y = " << finish.y << std::endl;
        return finish;
    }

    const std::pair<double, double>& GetSpeed() const noexcept {
        //std::cout << "Getting speed (" << speed_.first << ", " << speed_.second << ")" << std::endl;
        return speed_;
    }

    void SetSpeed(const std::pair<double, double>& speed) noexcept {
        //std::cout << "Setting speed to (" << speed.first << ", " << speed.second << ")" << std::endl;
        speed_ = speed;
    }

    const constants::Direction& GetDirection() const noexcept {
        return direction_;
    }

    void SetDirection(const constants::Direction& direction) noexcept {
        direction_ = direction;
    }

private:
    std::string name_;
    static uint32_t nextId;
    std::uint32_t dog_id = 0;
    Coordinates coordinates_;
    std::pair<double, double> speed_ = {0.0, 0.0};
    constants::Direction direction_ = constants::Direction::NORTH;
};


class GameSession {
public:

    using Id = util::Tagged<std::string, Map>;

    GameSession(const Map* map) : map_{map} {}

    void AddDog(std::shared_ptr<Dog> dog) {
        dog->SetCoordinateByPoint(map_->GetStartPointRoadMap());
        dogs_.insert(dog);        
    }

    void SetDogsCoordinatsByTime(int time_delta){
        for (auto& dog : dogs_) {
            //std::cout << std::fixed << std::setprecision(4);
            Coordinates start = dog->GetCoordinate();
            Dimension start_x = static_cast<Dimension>(start.x);
            Dimension start_y = static_cast<Dimension>(start.y);
            //std::cout << "start x = " << start.x << " y = " << start.y << std::endl;
            Coordinates calc_finish = dog->GetCoordinateByTime(time_delta);
            //std::cout << "finish x = " << calc_finish.x << " y = " << calc_finish.y << std::endl;
            Coordinates finish;
            constants::Direction direction = dog->GetDirection();
            if (direction == constants::Direction::EAST || direction == constants::Direction::WEST) {
                finish.y = start.y;
                if (const Road* road_cur = map_->GetHorRoad(start_x, start_y)) {
                    if (direction == constants::Direction::EAST) {
                        double road_x_end = static_cast<double>(road_cur->GetEnd().x);
                        if (calc_finish.x - road_x_end <= 0.4) {
                            finish.x = calc_finish.x;
                        } else {
                            finish.x = road_x_end + 0.4;
                        }
                    }
                    if (direction == constants::Direction::WEST) {
                        double road_x_begin = static_cast<double>(road_cur->GetStart().x);
                        if (road_x_begin - calc_finish.x <= 0.4) {
                            finish.x = calc_finish.x;
                        } else {
                            finish.x = road_x_begin - 0.4;
                        }
                    }
                } else {
                    if (direction == constants::Direction::EAST) {
                        if (calc_finish.x - static_cast<double>(start_x) <= 0.4) {
                            finish.x = calc_finish.x;
                        } else {
                            finish.x = static_cast<double>(start_x) + 0.4;
                        }
                    }
                    if (direction == constants::Direction::WEST) {
                        if (static_cast<double>(start_x) - calc_finish.x <= 0.4) {
                            finish.x = calc_finish.x;
                        } else {
                            finish.x = static_cast<double>(start_x) - 0.4;
                        }
                    }
                }
            } else if (direction == constants::Direction::NORTH || direction == constants::Direction::SOUTH) {
                finish.x = start.x;
                if (const Road* road_cur = map_->GetVerRoad(start_x, start_y)) {
                    if (direction == constants::Direction::SOUTH) {
                        double road_y_end = static_cast<double>(road_cur->GetEnd().y);
                        if (calc_finish.y - road_y_end <= 0.4) {
                            finish.y = calc_finish.y;
                        } else {
                            finish.y = road_y_end + 0.4;
                        }
                    }
                    if (direction == constants::Direction::NORTH) {
                        double road_y_begin = static_cast<double>(road_cur->GetStart().y);
                        if (road_y_begin - calc_finish.y <= 0.4) {
                            finish.y = calc_finish.y;
                        } else {
                            finish.y = road_y_begin - 0.4;
                        }
                    }
                } else {
                    if (direction == constants::Direction::SOUTH) {
                        if (calc_finish.y - static_cast<double>(start_y) <= 0.4) {
                            finish.y = calc_finish.y;
                        } else {
                            finish.y = static_cast<double>(start_y) + 0.4;
                        }
                    }
                    if (direction == constants::Direction::NORTH) {
                        if (static_cast<double>(start_y) - calc_finish.y <= 0.4) {
                            finish.y = calc_finish.y;
                        } else {
                            finish.y = static_cast<double>(start_y) - 0.4;
                        }
                    }
                }
            }

            //std::cout << "finish x = " << finish.x << " y = " << finish.y << std::endl;
            dog->SetCoordinate(finish);
            dog->SetSpeed({0, 0});

        }
    }

    const std::string& GetMapName() const noexcept {
        return map_->GetName();
    }

    const Map* GetMap() noexcept {
        return map_;
    }

    const Id& GetId() const noexcept {
        return map_->GetId();
    }

    const size_t GetDogsCount() const noexcept {
        return dogs_.size();
    }

    std::unordered_set<std::shared_ptr<Dog>>& GetDogs() noexcept {
        return dogs_;
    }


private:
    std::unordered_set<std::shared_ptr<Dog>> dogs_;
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

    void SetDefaultDogSpeed(double defaultDogSpeed){
        defaultDogSpeed_= defaultDogSpeed;
    }

    const double GetDefaultDogSpeed() {
        return defaultDogSpeed_;
    }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;

    std::vector<std::shared_ptr<GameSession>> sessions_;
    double defaultDogSpeed_ = 1;
};



}  // namespace model
