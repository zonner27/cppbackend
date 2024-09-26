#include "dog_serialization.h"

namespace serialization {

model::Dog DogRepr::Restore() const {
    model::Dog dog(id_, name_);
    dog.SetCoordinate(coordinates_);
    dog.SetSpeed(speed_);
    dog.SetDirection(direction_);
    dog.AddScore(score_);
    for (const auto& lost_obj_ser : bag_) {
        auto lost_object = std::make_shared<model::LostObject>(lost_obj_ser.Restore());
        dog.AddToBag(lost_object);
    }
    return dog;
}

} //serialization
