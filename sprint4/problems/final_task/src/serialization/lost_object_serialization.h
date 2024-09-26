#pragma once

#include "model_serialization.h"
#include "../model/lost_object.h"

namespace serialization {

class LostObjectRepr {
public:
    LostObjectRepr() = default;
    explicit LostObjectRepr(const model::LostObject& lost_object):
        id_(lost_object.GetId()),
        type_(lost_object.GetType()),
        coordinates_(lost_object.GetCoordinate()) {};

    [[nodiscard]] model::LostObject Restore() const;

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& type_;
        ar& coordinates_;
    }
private:
    std::uint32_t id_;
    size_t type_;
    geom::Point2D coordinates_;
};



} //serialization
