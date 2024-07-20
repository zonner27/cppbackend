#pragma once

#include "model.h"
#include "dog.h"
#include "maps.h"
#include "lost_object.h"
#include "loot_generator.h"
#include "../time/ticker.h"
#include "item_gatherer_provider.h"
#include "../events/geom.h"
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
            LootGeneratorConfig loot_gen_config, net::io_context& ioc)
        : map_{map}
        , loot_generator_(loot_gen::LootGenerator::TimeInterval(static_cast<uint32_t>(loot_gen_config.period * 1000)), loot_gen_config.probability)
        , game_session_strand_{std::make_shared<Strand>(net::make_strand(ioc))}
        , time_update_(time_update) {
    }

    void AddDog(std::shared_ptr<Dog> dog, bool randomize_spawn_points);

    const std::string& GetMapName() const noexcept;
    const Map* GetMap() noexcept;
    const Id& GetId() const noexcept;
    const size_t GetDogsCount() const noexcept;
    std::unordered_set<std::shared_ptr<Dog>>& GetDogs() noexcept;    
    std::unordered_set<std::shared_ptr<LostObject>>& GetLostObject() noexcept;
    size_t GetRandomTypeLostObject();

    void UpdateSessionByTime(const std::chrono::milliseconds& time_delta);
    void UpdateDogsCoordinatsByTime(const std::chrono::milliseconds& time_delta);
    void UpdateLootGenerationByTime(const std::chrono::milliseconds& time_delta);

    void PrintLostObjects(const std::unordered_set<std::shared_ptr<LostObject>>& lost_objects) {
        std::cout << "Current lost objects:" << std::endl;
        for (const auto& obj : lost_objects) {
            const geom::Point2D coord = obj->GetCoordinate();
            std::cout << "Lost object at coordinates (" << coord.x << ", " << coord.y << ")" << std::endl;
        }
    }

    void PrintBagContents(const std::vector<std::shared_ptr<LostObject>>& bag) {
        std::cout << "Current bag contents:" << std::endl;
        for (const auto& obj : bag) {
            const geom::Point2D coord = obj->GetCoordinate();
            std::cout << "Lost object at coordinates (" << coord.x << ", " << coord.y << ")" << std::endl;
        }
    }

    void Collector() {
        collision_detector::ItemGathererDogProvider provider;

        std::unordered_map<int, std::shared_ptr<LostObject>> lost_object_refs;
        std::vector<std::shared_ptr<Dog>> dog_refs;

        int id = 0;
        std::cout << "Adding lost objects to provider and map:" << std::endl;
        for (const auto& lost_object : lost_objects_) {
            const geom::Point2D coord = lost_object->GetCoordinate();
            provider.AddItem({coord, constants::WIDTH_ITEM, 0});
            lost_object_refs[id++] = lost_object;
            std::cout << "Added lost object with ID " << id - 1 << " at coordinates (" << coord.x << ", " << coord.y << ")" << std::endl;
        }

        for(const auto& office : map_->GetOffices()) {
            Point coord_point = office.GetPosition();
            provider.AddItem({{static_cast<double>(coord_point.x), static_cast<double>(coord_point.y)}, constants::WIDTH_BASE, 1});
        }

        for (const auto& dog : dogs_) {
            provider.AddGatherer(dog->GetGather());
            dog_refs.push_back(dog);
            std::cout << "Added dog" << std::endl;
        }

        auto events = collision_detector::FindGatherEvents(provider);
        std::cout << "Processing gather events:" << std::endl;
        for (auto event : events) {
            collision_detector::Item item = provider.GetItem(event.item_id);
            //collision_detector::Gatherer gatherer = provider.GetGatherer(event.gatherer_id);


            auto dog = dog_refs[event.gatherer_id];

            if (item.type == 0) {
                auto lost_object_it = lost_object_refs.find(event.item_id);
                if (lost_object_it == lost_object_refs.end() || lost_object_it->second == nullptr) {
                    continue;
                }

                auto lost_object = lost_object_it->second;
                if (dog->GetSizeBag() < map_->GetBagCapacity()) {
                    std::cout << "Dog can collect lost object with ID " << event.item_id << std::endl;
                    dog->AddToBag(lost_object);
                    lost_object_it->second = nullptr;
                    lost_objects_.erase(lost_object);

                    PrintLostObjects(lost_objects_);
                    PrintBagContents(dog->GetBag());
                } else {
                    std::cout << "Dog's bag is full, cannot collect lost object with ID " << event.item_id << std::endl;
                }
            } else if (item.type == 1) {
                for (const auto& lost_obj : dog->GetBag()) {
                    dog->AddScore(lost_obj->GetType());
                }
                dog->ClearBag();
            }
        }

    }

    std::shared_ptr<Strand> GetSessionStrand();
    void Run();

private:
    std::unordered_set<std::shared_ptr<Dog>> dogs_;
    std::unordered_set<std::shared_ptr<LostObject>> lost_objects_;
    loot_gen::LootGenerator loot_generator_;
    loot_gen::LootGenerator::TimeInterval time_update_;
    const Map* map_;
    std::shared_ptr<Strand> game_session_strand_;
    std::shared_ptr<time_tiker::Ticker> ticker_;
    std::shared_ptr<time_tiker::Ticker> loot_ticker_;
};

} //namespace model
