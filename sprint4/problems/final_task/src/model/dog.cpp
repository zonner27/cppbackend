#include "dog.h"

namespace model {

uint32_t Dog::nextId = 0;

const std::string &Dog::GetName() const noexcept {
    return name_;
}

const uint32_t Dog::GetId() const noexcept {
    return dog_id;
}

const geom::Point2D &Dog::GetCoordinate() const noexcept {
    return coordinates_;
}

void Dog::SetCoordinate(const geom::Point2D &coordinates) noexcept {
    gatherer_.start_pos = coordinates_;
    coordinates_ = coordinates;
    gatherer_.end_pos = coordinates_;
}

void Dog::SetCoordinateByPoint(const Point &point) noexcept {
    coordinates_.x = static_cast<double>(point.x);
    coordinates_.y = static_cast<double>(point.y);
}

const geom::Point2D Dog::GetCoordinateByTime(int time_delta) const noexcept {
    geom::Point2D finish;
    finish.x = coordinates_.x + speed_.first * static_cast<double>(time_delta) / 1000;
    finish.y = coordinates_.y + speed_.second * static_cast<double>(time_delta) / 1000;
    return finish;
}

const std::pair<double, double> &Dog::GetSpeed() const noexcept {
    return speed_;
}

void Dog::SetSpeed(const std::pair<double, double> &speed) noexcept {
    speed_ = speed;
    dog_is_stoped_ = (speed_ == std::make_pair(0.0, 0.0));
}

const constants::Direction &Dog::GetDirection() const noexcept {
    return direction_;
}

void Dog::SetDirection(const constants::Direction &direction) noexcept {
    direction_ = direction;
}

std::vector<std::shared_ptr<LostObject> > Dog::GetBag() const noexcept {
    return bag_;
}

void Dog::AddToBag(std::shared_ptr<LostObject> lostobject) {
    bag_.push_back(lostobject);
}

void Dog::ClearBag() {
    bag_.clear();
}

void Dog::AddScore(uint32_t score) {
    score_ += score;;
}

const uint32_t Dog::GetScore() const noexcept{
    return score_;
}

const size_t Dog::GetSizeBag() const noexcept {
    return bag_.size();
}

const collision_detector::Gatherer &Dog::GetGather() const noexcept{
    return gatherer_;
}

void Dog::SetRetirementTime(uint32_t retired_time_in_ms) {
    retirementTime_ = std::chrono::milliseconds(retired_time_in_ms);
}

void Dog::AddTime(const std::chrono::milliseconds &time_delta) noexcept {
    game_time_ += time_delta;

    if (dog_is_stoped_) {
        stoped_time_ += time_delta;
    } else {
        stoped_time_ = std::chrono::milliseconds(0);
    }
    if (stoped_time_ >= retirementTime_) {
        dog_is_retired_ = true;
    }
}

bool Dog::IsRetired() {
    return dog_is_retired_;
}

uint32_t Dog::GetGameTime() {
    return game_time_.count();
}

} // namespace model
