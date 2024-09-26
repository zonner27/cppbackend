#pragma once

#include "model_serialization.h"
#include "../model/game_session.h"
#include "lost_object_serialization.h"
#include "dog_serialization.h"

namespace serialization {

class GameSessionResp {
public:
    GameSessionResp() = default;
    GameSessionResp(
        model::GameSession& game_session) :
            map_id_(*(game_session.GetMap()->GetId())) {

        const auto& lost_objects = game_session.GetLostObjects();
        for (const auto& lost_object : lost_objects) {
            lost_objects_repr_.emplace_back(*lost_object);
        }

        const auto& dogs = game_session.GetDogs();
        for (const auto& dog : dogs) {
            dogs_repr_.emplace_back(*dog);
        }
    };

    [[nodiscard]] model::Map::Id RestoreMapId() const;
    [[nodiscard]] const std::vector<LostObjectRepr>& GetLostObjectsResp() const;
    [[nodiscard]] const std::vector<DogRepr>& GetDogsResp() const;

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& map_id_;
        ar& lost_objects_repr_;
        ar& dogs_repr_;
    }
private:
    std::string map_id_;
    std::vector<LostObjectRepr> lost_objects_repr_;
    std::vector<DogRepr> dogs_repr_;
};

} //serialization
