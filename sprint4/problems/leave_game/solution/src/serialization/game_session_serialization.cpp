#include "game_session_serialization.h"

namespace serialization {

model::Map::Id GameSessionResp::RestoreMapId() const {
    return model::Map::Id(map_id_);
}

const std::vector<LostObjectRepr> &GameSessionResp::GetLostObjectsResp() const {
    return lost_objects_repr_;
}

const std::vector<DogRepr> &GameSessionResp::GetDogsResp() const {
    return dogs_repr_;
}


} //serialization
