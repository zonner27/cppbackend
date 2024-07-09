#pragma once
#include "model.h"

namespace model {

class LostObject{
public:
    LostObject() : id_{nextId++} {}

    const uint32_t GetId() const noexcept;

    const Coordinates& GetCoordinate() const noexcept;
    void SetCoordinate(const Coordinates& coordinates) noexcept;
    void SetCoordinateByPoint(const Point& point) noexcept;

    size_t GetType() const;
    void SetType(size_t type);



private:
    static uint32_t nextId;
    std::uint32_t id_ {0};
    size_t type_ {0};
    Coordinates coordinates_ {0.0, 0.0};

};


} //namespace model
