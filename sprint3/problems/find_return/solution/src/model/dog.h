#pragma once
#include "model.h"
#include "lost_object.h"
#include "../events/collision_detector.h"
#include "../events/geom.h"
#include "../constants.h"

namespace model {

class Dog{
public:
    Dog(std::string& name) : name_{name}, dog_id{nextId++} {}
    const std::string& GetName() const noexcept;
    const uint32_t GetId() const noexcept;

    const geom::Point2D& GetCoordinate() const noexcept;
    void SetCoordinate(const geom::Point2D& coordinates) noexcept;
    void SetCoordinateByPoint(const Point& point) noexcept;
    geom::Point2D GetCoordinateByTime(int time_delta) noexcept;

    const std::pair<double, double>& GetSpeed() const noexcept;
    void SetSpeed(const std::pair<double, double>& speed) noexcept;

    const constants::Direction& GetDirection() const noexcept;
    void SetDirection(const constants::Direction& direction) noexcept;

    std::vector<std::shared_ptr<LostObject>> GetBag() const noexcept {
        return bag_;
    }
    void AddToBag(std::shared_ptr<LostObject> lostobject) {
        bag_.push_back(lostobject);
    }

    void ClearBag() {
        bag_.clear();
    }

    void AddScore(std::uint32_t score) {
        score_ += score;;
    }

    const uint32_t GetScore() const noexcept{
        return score_;
    }

    const size_t GetSizeBag() const noexcept {
        return bag_.size();
    }

    const collision_detector::Gatherer& GetGather() const noexcept;

private:
    std::string name_;
    static uint32_t nextId;
    std::uint32_t dog_id = 0;
    geom::Point2D coordinates_ {0.0, 0.0};
    std::pair<double, double> speed_ {0.0, 0.0};
    collision_detector::Gatherer gatherer_{{0, 0}, {0, 0}, constants::WIDTH_PLAYER};
    constants::Direction direction_ = constants::Direction::NORTH;
    std::vector<std::shared_ptr<LostObject>> bag_;
    std::uint32_t score_ = 0;
};

} // namespace model
