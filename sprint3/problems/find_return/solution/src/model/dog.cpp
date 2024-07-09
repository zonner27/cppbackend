#include "dog.h"

namespace model {

uint32_t Dog::nextId = 0;

const std::string &Dog::GetName() const noexcept {
    return name_;
}

const uint32_t Dog::GetId() const noexcept {
    return dog_id;
}

const Coordinates &Dog::GetCoordinate() const noexcept {
    return coordinates_;
}

void Dog::SetCoordinate(const Coordinates &coordinates) noexcept {
    coordinates_ = coordinates;
}

void Dog::SetCoordinateByPoint(const Point &point) noexcept {
    coordinates_.x = static_cast<double>(point.x);
    coordinates_.y = static_cast<double>(point.y);
}

Coordinates Dog::GetCoordinateByTime(int time_delta) noexcept {
    Coordinates finish;
    finish.x = coordinates_.x + speed_.first * static_cast<double>(time_delta) / 1000;
    finish.y = coordinates_.y + speed_.second * static_cast<double>(time_delta) / 1000;
    return finish;
}

const std::pair<double, double> &Dog::GetSpeed() const noexcept {
    return speed_;
}

void Dog::SetSpeed(const std::pair<double, double> &speed) noexcept {
    speed_ = speed;
}

const constants::Direction &Dog::GetDirection() const noexcept {
    return direction_;
}

void Dog::SetDirection(const constants::Direction &direction) noexcept {
    direction_ = direction;
}

const std::vector<LostObject> &Dog::GetBag() const {
    return bag_;
}

} // namespace model
