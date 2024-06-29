#pragma once
#include "model.h"

namespace model {

class Dog{
public:
    Dog(std::string& name) : name_{name}, dog_id{nextId++} {}
    const std::string& GetName() const noexcept;
    const uint32_t GetId() const noexcept;

    const Coordinates& GetCoordinate() const noexcept;
    void SetCoordinate(const Coordinates& coordinates) noexcept;
    void SetCoordinateByPoint(const Point& point) noexcept;
    Coordinates GetCoordinateByTime(int time_delta) noexcept;

    const std::pair<double, double>& GetSpeed() const noexcept;
    void SetSpeed(const std::pair<double, double>& speed) noexcept;

    const constants::Direction& GetDirection() const noexcept;
    void SetDirection(const constants::Direction& direction) noexcept;

private:
    std::string name_;
    static uint32_t nextId;
    std::uint32_t dog_id = 0;
    Coordinates coordinates_ {0.0, 0.0};
    std::pair<double, double> speed_ {0.0, 0.0};
    constants::Direction direction_ = constants::Direction::NORTH;
};

} // namespace model
