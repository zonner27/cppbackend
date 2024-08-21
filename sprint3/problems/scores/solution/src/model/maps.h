#pragma once

#include "model.h"

namespace model {

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using RoadMap = std::map<int, Roads>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;
    using LootTypes = std::vector<LootType>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept;
    const std::string& GetName() const noexcept;
    const Buildings& GetBuildings() const noexcept;
    const Roads& GetRoads() const noexcept;
    const Offices& GetOffices() const noexcept;
    const LootTypes& GetLootTypes() const noexcept;
    void AddRoad(const Road& road);
    const Road* GetHorRoad(Coord x, Coord y) const;
    const Road* GetVerRoad(Coord x, Coord y) const;

    void AddBuilding(const Building& building);
    void AddOffice(Office office);    
    void AddLootType(const LootType& loot_type);

    void SetDogSpeed(double dogSpeed);
    const double GetDogSpeed() const noexcept;

    void SetBagCapaccity(int bagCapacity);
    const int GetBagCapacity() const noexcept;

    static int GetRandomNumber(int min, int max);
    Point GetRandomPointOnRoad(const Road& road) const;
    Point GetRandomPointRoadMap() const;

    const Point& GetStartPointRoadMap() const noexcept;
    std::uint32_t GetScoreByLootType(size_t id);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Point start_roads_point_{-1, -1};
    RoadMap hor_roads_;
    RoadMap ver_roads_;
    Buildings buildings_;
    LootTypes loot_types_;
    double dogSpeed_;
    int bagCapacity_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

}  // namespace model
