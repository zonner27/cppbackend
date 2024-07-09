#include "lost_object.h"

namespace model {
uint32_t LostObject::nextId = 0;

const uint32_t LostObject::GetId() const noexcept {
    return id_;
}

const Coordinates &LostObject::GetCoordinate() const noexcept {
    return coordinates_;
}

void LostObject::SetCoordinate(const Coordinates &coordinates) noexcept {
    coordinates_ = coordinates;
}

void LostObject::SetCoordinateByPoint(const Point &point) noexcept {
    coordinates_.x = static_cast<double>(point.x);
    coordinates_.y = static_cast<double>(point.y);
}

size_t LostObject::GetType() const {
    return type_;
}

void LostObject::SetType(size_t type) {
    type_ = type;
}


} //namespace model
