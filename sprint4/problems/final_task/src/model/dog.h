#pragma once
#include "model.h"
#include "lost_object.h"
#include "../events/collision_detector.h"
#include "../events/geom.h"
#include "../constants.h"

#include <chrono>

namespace model {

class Dog{
    inline static std::chrono::milliseconds retirementTime_{60000};
public:
    Dog(std::string& name) : name_{name}, dog_id{nextId++} {}

    Dog(uint32_t id, std::string name) :
        dog_id(id),
        name_(name) {
        if (id >= nextId) {
            nextId = id + 1;
        }
    }

    const std::string& GetName() const noexcept;
    const uint32_t GetId() const noexcept;

    const geom::Point2D& GetCoordinate() const noexcept;
    void SetCoordinate(const geom::Point2D& coordinates) noexcept;
    void SetCoordinateByPoint(const Point& point) noexcept;
    const geom::Point2D GetCoordinateByTime(int time_delta) const noexcept;

    const std::pair<double, double>& GetSpeed() const noexcept;
    void SetSpeed(const std::pair<double, double>& speed) noexcept;

    const constants::Direction& GetDirection() const noexcept;
    void SetDirection(const constants::Direction& direction) noexcept;

    std::vector<std::shared_ptr<LostObject>> GetBag() const noexcept;
    void AddToBag(std::shared_ptr<LostObject> lostobject);
    void ClearBag();
    const size_t GetSizeBag() const noexcept;
    void AddScore(std::uint32_t score);
    const uint32_t GetScore() const noexcept;
    const collision_detector::Gatherer& GetGather() const noexcept;

    static void SetRetirementTime(uint32_t retired_time_in_ms);
    void AddTime(const std::chrono::milliseconds& time_delta) noexcept;
    bool IsRetired();
    uint32_t GetGameTime();

private:
    static uint32_t nextId;
    std::uint32_t dog_id = 0;
    std::string name_;
    geom::Point2D coordinates_ {0.0, 0.0};
    std::pair<double, double> speed_ {0.0, 0.0};
    collision_detector::Gatherer gatherer_{{0, 0}, {0, 0}, constants::WIDTH_PLAYER};
    constants::Direction direction_ = constants::Direction::NORTH;
    std::vector<std::shared_ptr<LostObject>> bag_;
    std::uint32_t score_{0};
    bool dog_is_stoped_{true};
    bool dog_is_retired_{false};
    std::chrono::milliseconds stoped_time_{0};
    std::chrono::milliseconds game_time_{0};
};


} // namespace model
