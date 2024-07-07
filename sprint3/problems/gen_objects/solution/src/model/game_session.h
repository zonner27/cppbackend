#pragma once

#include "model.h"
#include "dog.h"
#include "maps.h"
#include "lost_object.h"
#include "loot_generator.h"
#include <cmath>
#include <random>
#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>

namespace net = boost::asio;

namespace model {

class GameSession : public std::enable_shared_from_this<GameSession> {
public:

    using Id = util::Tagged<std::string, Map>;
    using Strand = net::strand<net::io_context::executor_type>;

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

    void UpdateSessionByTime(std::chrono::milliseconds time_delta, std::shared_ptr<Strand>& api_strand) {
        UpdateDogsCoordinatsByTime(time_delta);
        UpdateLootGeneration(time_delta, api_strand);
    }

    size_t GetRandomTypeLostObject() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dis(0, map_->GetLootTypes().size() - 1);

        return dis(gen);
    }

    void UpdateLootGeneration(std::chrono::milliseconds time_delta, std::shared_ptr<Strand>& api_strand) {
        unsigned loot_count = lost_objects_.size();
        unsigned looter_count = dogs_.size();

        net::dispatch(*api_strand, [self = shared_from_this(), &time_delta, &loot_count, &looter_count]() mutable {
            unsigned new_loot_count = self->loot_generator_.Generate(time_delta, loot_count, looter_count);

            for (unsigned i = 0; i < new_loot_count; ++i) {
                auto lost_object = std::make_shared<LostObject>();
                lost_object->SetCoordinateByPoint(self->map_->GetRandomPointRoadMap());
                lost_object->SetType(self->GetRandomTypeLostObject());
                self->lost_objects_.insert(lost_object);
            }
        });
    }



private:
    std::unordered_set<std::shared_ptr<Dog>> dogs_;
    std::unordered_set<std::shared_ptr<LostObject>> lost_objects_;
    loot_gen::LootGenerator loot_generator_;
    const Map* map_;
};

} //namespace model
