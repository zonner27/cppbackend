#pragma once
//Алгоритм добавления собаки в игровой сеанс будет выглядеть так.
//Найти игровой сеанс, соответствующий карте, на которой хочет играть клиент.
//Внутри игрового сеанса добавить нового пса с указанным именем и сгенерированным id.
#include "model.h"


namespace model {

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
//        std::cout << std::fixed << std::setprecision(4);
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
    Coordinates coordinates_ {0.0, 0.0};
    std::pair<double, double> speed_ {0.0, 0.0};
    constants::Direction direction_ = constants::Direction::NORTH;
};

} // namespace model
