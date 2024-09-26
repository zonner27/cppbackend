#include "game_session.h"

namespace model {

void GameSession::AddDog(std::shared_ptr<Dog> dog, bool randomize_spawn_points) {
    if (randomize_spawn_points) {
        Point dog_coord = map_->GetRandomPointRoadMap();
        dog->SetCoordinateByPoint(dog_coord);
    } else {       
       dog->SetCoordinateByPoint(map_->GetStartPointRoadMap());
    }
    dogs_.insert(dog);
}

void GameSession::AddDog(std::shared_ptr<Dog> dog) {
    dogs_.insert(dog);
}

const std::string &model::GameSession::GetMapName() const noexcept {
    return map_->GetName();
}

const Map *GameSession::GetMap() noexcept {
    return map_;
}

const size_t GameSession::GetDogsCount() const noexcept {
    return dogs_.size();
}

std::unordered_set<std::shared_ptr<Dog>>& GameSession::GetDogs() noexcept {
    return dogs_;
}

std::unordered_set<std::shared_ptr<LostObject>> &GameSession::GetLostObjects() noexcept {
    return lost_objects_;
}

void GameSession::AddLostObject(std::shared_ptr<LostObject> &lost_object) {
    lost_objects_.insert(lost_object);
}

void GameSession::UpdateSessionByTime(const std::chrono::milliseconds& time_delta) {

    UpdateDogsCoordinatsByTime(time_delta);
    UpdateLootGenerationByTime(time_delta);
    Collector();
    DeleteRetiredDog();
}

void GameSession::UpdateDogsCoordinatsByTime(const std::chrono::milliseconds& time_delta_ms){

    int time_delta = static_cast<int>(time_delta_ms.count());

    for (auto& dog : dogs_) {
        dog->AddTime(time_delta_ms);
        geom::Point2D start = dog->GetCoordinate();
        Dimension start_x = static_cast<Dimension>(std::round(start.x));
        Dimension start_y = static_cast<Dimension>(std::round(start.y));

        geom::Point2D calc_finish = dog->GetCoordinateByTime(time_delta);
        geom::Point2D finish;
        constants::Direction direction = dog->GetDirection();
        if (direction == constants::Direction::EAST || direction == constants::Direction::WEST) {
            finish.y = start.y;
            if (const Road* road_cur = map_->GetHorRoad(start_x, start_y)) {
                if (direction == constants::Direction::EAST) {
                    double road_x_end = static_cast<double>(road_cur->GetEnd().x);
                    if (calc_finish.x - road_x_end <= constants::MAXDISTANCEFROMCENTER) {
                        finish.x = calc_finish.x;
                    } else {
                        finish.x = road_x_end + constants::MAXDISTANCEFROMCENTER;
                        dog->SetSpeed({0, 0});
                    }
                }
                if (direction == constants::Direction::WEST) {
                    double road_x_begin = static_cast<double>(road_cur->GetStart().x);
                    if (road_x_begin - calc_finish.x <= constants::MAXDISTANCEFROMCENTER) {
                        finish.x = calc_finish.x;
                    } else {
                        finish.x = road_x_begin - constants::MAXDISTANCEFROMCENTER;
                        dog->SetSpeed({0, 0});
                    }
                }
            } else {
                if (direction == constants::Direction::EAST) {
                    if (calc_finish.x - static_cast<double>(start_x) <= constants::MAXDISTANCEFROMCENTER) {
                        finish.x = calc_finish.x;
                    } else {
                        finish.x = static_cast<double>(start_x) + constants::MAXDISTANCEFROMCENTER;
                        dog->SetSpeed({0, 0});
                    }
                }
                if (direction == constants::Direction::WEST) {
                    if (static_cast<double>(start_x) - calc_finish.x <= constants::MAXDISTANCEFROMCENTER) {
                        finish.x = calc_finish.x;
                    } else {
                        finish.x = static_cast<double>(start_x) - constants::MAXDISTANCEFROMCENTER;
                        dog->SetSpeed({0, 0});
                    }
                }
            }
        } else if (direction == constants::Direction::NORTH || direction == constants::Direction::SOUTH) {
            finish.x = start.x;
            if (const Road* road_cur = map_->GetVerRoad(start_x, start_y)) {
                if (direction == constants::Direction::SOUTH) {
                    double road_y_end = static_cast<double>(road_cur->GetEnd().y);
                    if (calc_finish.y - road_y_end <= constants::MAXDISTANCEFROMCENTER) {
                        finish.y = calc_finish.y;
                    } else {
                        finish.y = road_y_end + constants::MAXDISTANCEFROMCENTER;
                        dog->SetSpeed({0, 0});
                    }
                }
                if (direction == constants::Direction::NORTH) {
                    double road_y_begin = static_cast<double>(road_cur->GetStart().y);
                    if (road_y_begin - calc_finish.y <= constants::MAXDISTANCEFROMCENTER) {
                        finish.y = calc_finish.y;
                    } else {
                        finish.y = road_y_begin - constants::MAXDISTANCEFROMCENTER;
                        dog->SetSpeed({0, 0});
                    }
                }
            } else {
                if (direction == constants::Direction::SOUTH) {
                    if (calc_finish.y - static_cast<double>(start_y) <= constants::MAXDISTANCEFROMCENTER) {
                        finish.y = calc_finish.y;
                    } else {
                        finish.y = static_cast<double>(start_y) + constants::MAXDISTANCEFROMCENTER;
                        dog->SetSpeed({0, 0});
                    }
                }
                if (direction == constants::Direction::NORTH) {
                    if (static_cast<double>(start_y) - calc_finish.y <= constants::MAXDISTANCEFROMCENTER) {
                        finish.y = calc_finish.y;
                    } else {
                        finish.y = static_cast<double>(start_y) - constants::MAXDISTANCEFROMCENTER;
                        dog->SetSpeed({0, 0});
                    }
                }
            }
        }
        dog->SetCoordinate(finish);        
    }
}

void GameSession::UpdateLootGenerationByTime(const std::chrono::milliseconds& time_delta) {

    unsigned loot_count = lost_objects_.size();
    unsigned looter_count = dogs_.size();

    unsigned new_loot_count = loot_generator_.Generate(time_delta, loot_count, looter_count);

    for (unsigned i = 0; i < new_loot_count; ++i) {
        auto lost_object = std::make_shared<LostObject>();
        Point loot_coord = map_->GetRandomPointRoadMap();
        lost_object->SetCoordinateByPoint(loot_coord);
        lost_object->SetType(GetRandomTypeLostObject());
        lost_objects_.insert(lost_object);
    }
}

void GameSession::Collector() {
    collision_detector::ItemGathererDogProvider provider;

    std::unordered_map<int, std::shared_ptr<LostObject>> lost_object_refs;
    std::vector<std::shared_ptr<Dog>> dog_refs;

    int id = 0;
    for (const auto& lost_object : lost_objects_) {
        const geom::Point2D coord = lost_object->GetCoordinate();
        provider.AddItem({coord, constants::WIDTH_ITEM, 0});
        lost_object_refs[id++] = lost_object;
    }

    for(const auto& office : map_->GetOffices()) {
        Point coord_point = office.GetPosition();
        provider.AddItem({{static_cast<double>(coord_point.x), static_cast<double>(coord_point.y)}, constants::WIDTH_BASE, 1});
    }

    for (const auto& dog : dogs_) {
        provider.AddGatherer(dog->GetGather());
        dog_refs.push_back(dog);
    }

    auto events = collision_detector::FindGatherEvents(provider);
    for (auto event : events) {
        collision_detector::Item item = provider.GetItem(event.item_id);
        auto dog = dog_refs[event.gatherer_id];

        if (item.type == 0) {
            auto lost_object_it = lost_object_refs.find(event.item_id);
            if (lost_object_it == lost_object_refs.end() || lost_object_it->second == nullptr) {
                continue;
            }

            auto lost_object = lost_object_it->second;
            if (dog->GetSizeBag() < map_->GetBagCapacity()) {
                dog->AddToBag(lost_object);
                lost_object_it->second = nullptr;
                lost_objects_.erase(lost_object);
            }
        } else if (item.type == 1) {
            for (const auto& lost_obj : dog->GetBag()) {
                dog->AddScore(lost_obj->GetType());
            }
            dog->ClearBag();
        }
    }
}

size_t GameSession::GetRandomTypeLostObject() {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<size_t> dis(0, map_->GetLootTypes().size() - 1);
    return dis(eng);
}

std::shared_ptr<GameSession::Strand> GameSession::GetSessionStrand() {
    return game_session_strand_;
}

void GameSession::Run() {

    if(time_update_.count() != 0) {
        ticker_ = std::make_shared<time_tiker::Ticker>(
                    *game_session_strand_,
                    time_update_,
                    [self_weak = weak_from_this()](std::chrono::milliseconds delta) {
            if(auto self = self_weak.lock()) {
                self->UpdateSessionByTime(delta);
            }
        }
        );
        ticker_->Start();
    }
}

void GameSession::DeleteRetiredDog() {
    std::vector<domain::RetiredPlayers> retired_players;

    for (auto it = dogs_.begin(); it != dogs_.end();) {
        if ((*it)->IsRetired()) {
            retired_players.emplace_back(domain::RetiredPlayersId::New(),
                                         (*it)->GetName(),
                                         (*it)->GetId(),
                                         (*it)->GetScore(),
                                         (*it)->GetGameTime());
            it = dogs_.erase(it);
        } else {
            ++it;
        }
    }

    if (retired_players.empty()) {
        return;
    }

    retired_players_signal_(std::move(retired_players));
}

boost::signals2::connection GameSession::ConnectRetiredPlayersSignal(RetiredPlayersSignal::slot_type slot) {
    return retired_players_signal_.connect(slot);
}

const GameSession::Id &GameSession::GetId() const noexcept {
    return map_->GetId();
}

} //namespace model

