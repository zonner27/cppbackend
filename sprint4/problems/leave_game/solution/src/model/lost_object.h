#pragma once
#include "model.h"
#include "../events/geom.h"

namespace model {

class LostObject{
public:
    LostObject() : id_{nextId++} {}
    LostObject(std::uint32_t id) : id_{id} {}

    const uint32_t GetId() const noexcept;
    void SetId(std::uint32_t id) noexcept;
    const geom::Point2D& GetCoordinate() const noexcept;
    void SetCoordinate(const geom::Point2D& coordinates) noexcept;
    void SetCoordinateByPoint(const Point& point) noexcept;
    size_t GetType() const;
    void SetType(size_t type);

private:
    static uint32_t nextId;
    std::uint32_t id_ {0};
    size_t type_ {0};
    geom::Point2D coordinates_ {0.0, 0.0};

};


} //namespace model
