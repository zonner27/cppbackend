#pragma once

#include "model_serialization.h"
#include "lost_object_serialization.h"
#include "../model/dog.h"

namespace serialization {

class DogRepr {
public:
    DogRepr() = default;

    explicit DogRepr(const model::Dog& dog)
        : id_(dog.GetId())
        , name_(dog.GetName())
        , coordinates_(dog.GetCoordinate())
        , speed_(dog.GetSpeed())
        , direction_(dog.GetDirection())
        , score_(dog.GetScore()) {
        for (const auto& lost_object : dog.GetBag()) {
            bag_.emplace_back(*lost_object);
        }
    }

    [[nodiscard]] model::Dog Restore() const;

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& name_;
        ar& coordinates_;
        ar& speed_;
        ar& direction_;
        ar& bag_;
        ar& score_;
    }

private:
    std::uint32_t id_;
    std::string name_{};
    geom::Point2D coordinates_ {0.0, 0.0};
    std::pair<double, double> speed_ {0.0, 0.0};
    constants::Direction direction_ = constants::Direction::NORTH;
    std::vector<LostObjectRepr> bag_;
    std::uint32_t score_ = 0;
};


} //serialization
