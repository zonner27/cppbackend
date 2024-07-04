#pragma once

#include "model.h"
#include "dog.h"
#include "maps.h"
#include "lost_object.h"
#include "loot_generator.h"
#include <cmath>
#include <random>

namespace model {

class GameSession {
public:

    using Id = util::Tagged<std::string, Map>;

    GameSession(const Map* map,
                loot_gen::LootGenerator::TimeInterval time_update,
                LootGeneratorConfig loot_gen_config)
            : map_{map}
            , loot_generator_(loot_gen::LootGenerator::TimeInterval(static_cast<uint32_t>(loot_gen_config.period * 1000)), loot_gen_config.probability) {

        }

    void AddDog(std::shared_ptr<Dog> dog, bool randomize_spawn_points);
    void UpdateDogsCoordinatsByTime(std::chrono::milliseconds time_delta);
    const std::string& GetMapName() const noexcept;
    const Map* GetMap() noexcept;
    const Id& GetId() const noexcept;
    const size_t GetDogsCount() const noexcept;
    std::unordered_set<std::shared_ptr<Dog>>& GetDogs() noexcept;    
    std::unordered_set<std::shared_ptr<LostObject>>& GetLostObject() noexcept;

    void UpdateSessionByTime(std::chrono::milliseconds time_delta) {
        UpdateDogsCoordinatsByTime(time_delta);
        UpdateLootGeneration(time_delta);
    }

    size_t GetRandomTypeLostObject() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dis(0, map_->GetLootTypes().size() - 1);

        return dis(gen);
    }

    void UpdateLootGeneration(std::chrono::milliseconds time_delta) {
        unsigned loot_count = lost_objects_.size();
        unsigned looter_count = dogs_.size();

        unsigned new_loot_count = loot_generator_.Generate(time_delta, loot_count, looter_count);

        for (unsigned i = 0; i < new_loot_count; ++i) {
            auto lost_object = std::make_shared<LostObject>();
            lost_object->SetCoordinateByPoint(map_->GetRandomPointRoadMap());
            lost_object->SetType(GetRandomTypeLostObject());
            lost_objects_.insert(lost_object);
        }
    }



private:
    std::unordered_set<std::shared_ptr<Dog>> dogs_;
    std::unordered_set<std::shared_ptr<LostObject>> lost_objects_;
    loot_gen::LootGenerator loot_generator_;
    const Map* map_;
};

} //namespace model
