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

class GameSession {
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
    void UpdateDogsCoordinatsByTime(std::chrono::milliseconds time_delta, std::shared_ptr<Strand>& api_strand);
    const std::string& GetMapName() const noexcept;
    const Map* GetMap() noexcept;
    const Id& GetId() const noexcept;
    const size_t GetDogsCount() const noexcept;
    std::unordered_set<std::shared_ptr<Dog>>& GetDogs() noexcept;    
    std::unordered_set<std::shared_ptr<LostObject>>& GetLostObject() noexcept;
    void UpdateSessionByTime(std::chrono::milliseconds time_delta, std::shared_ptr<Strand>& api_strand);
    size_t GetRandomTypeLostObject();
    void UpdateLootGeneration(std::chrono::milliseconds time_delta, std::shared_ptr<Strand>& api_strand);

private:
    std::unordered_set<std::shared_ptr<Dog>> dogs_;
    std::unordered_set<std::shared_ptr<LostObject>> lost_objects_;
    loot_gen::LootGenerator loot_generator_;
    const Map* map_;
};

} //namespace model
