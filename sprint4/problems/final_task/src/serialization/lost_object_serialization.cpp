#include "lost_object_serialization.h"


namespace serialization {

model::LostObject LostObjectRepr::Restore() const {
    model::LostObject lost_object(id_);
    lost_object.SetType(type_);
    lost_object.SetCoordinate(coordinates_);
    return lost_object;
}

} //serialization
