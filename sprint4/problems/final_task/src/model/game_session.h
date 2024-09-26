#pragma once

#include "model.h"
#include "dog.h"
#include "maps.h"
#include "lost_object.h"
#include "loot_generator.h"
#include "../time/ticker.h"
#include "item_gatherer_provider.h"
#include "../events/geom.h"
#include "../database/retired_players.h"

#include <cmath>
#include <random>
#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/signals2.hpp>
#include <set>

namespace net = boost::asio;

namespace model {

class GameSession : public std::enable_shared_from_this<GameSession> {
public:

    using Id = util::Tagged<std::string, Map>;
    using Strand = net::strand<net::io_context::executor_type>;
    using RetiredPlayersSignal = boost::signals2::signal<void(std::vector<domain::RetiredPlayers>)>;

    GameSession(const Map* map,
            loot_gen::LootGenerator::TimeInterval time_update,
            LootGeneratorConfig loot_gen_config, net::io_context& ioc)
        : map_{map}
        , loot_generator_(loot_gen::LootGenerator::TimeInterval(static_cast<uint32_t>(loot_gen_config.period * 1000)), loot_gen_config.probability)
        , game_session_strand_{std::make_shared<Strand>(net::make_strand(ioc))}
        , time_update_(time_update) {
    }

    void AddDog(std::shared_ptr<Dog> dog, bool randomize_spawn_points);
    void AddDog(std::shared_ptr<Dog> dog);
    const std::string& GetMapName() const noexcept;
    const Map* GetMap() noexcept;
    const Id& GetId() const noexcept;
    const size_t GetDogsCount() const noexcept;
    std::unordered_set<std::shared_ptr<Dog>>& GetDogs() noexcept;
    std::unordered_set<std::shared_ptr<LostObject>>& GetLostObjects() noexcept;
    void AddLostObject(std::shared_ptr<LostObject>& lost_object);
    size_t GetRandomTypeLostObject();
    void UpdateSessionByTime(const std::chrono::milliseconds& time_delta);
    void UpdateDogsCoordinatsByTime(const std::chrono::milliseconds& time_delta);
    void UpdateLootGenerationByTime(const std::chrono::milliseconds& time_delta);
    void Collector();
    std::shared_ptr<Strand> GetSessionStrand();
    void Run();
    void DeleteRetiredDog();
    boost::signals2::connection ConnectRetiredPlayersSignal(RetiredPlayersSignal::slot_type slot);

private:
    std::unordered_set<std::shared_ptr<Dog>> dogs_;
    std::unordered_set<std::shared_ptr<LostObject>> lost_objects_;
    loot_gen::LootGenerator loot_generator_;
    loot_gen::LootGenerator::TimeInterval time_update_;
    const Map* map_;
    std::shared_ptr<Strand> game_session_strand_;
    std::shared_ptr<time_tiker::Ticker> ticker_;
    std::shared_ptr<time_tiker::Ticker> loot_ticker_;
    RetiredPlayersSignal retired_players_signal_;
};

} //namespace model
