#include "game_session.h"

#include <iostream>  //del

namespace model {

void GameSession::AddDog(std::shared_ptr<Dog> dog, bool randomize_spawn_points) {
    if (randomize_spawn_points) {
        Point dog_coord = map_->GetRandomPointRoadMap();
        //std::cout << "set dog coord = " << dog_coord.x << " y = " << dog_coord.y << std::endl;
        dog->SetCoordinateByPoint(dog_coord);
    } else {
       dog->SetCoordinateByPoint(map_->GetStartPointRoadMap());
    }
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

std::unordered_set<std::shared_ptr<Dog> > &GameSession::GetDogs() noexcept {
    return dogs_;
}

std::unordered_set<std::shared_ptr<LostObject> > &GameSession::GetLostObject() noexcept {
    return lost_objects_;
}

void GameSession::UpdateSessionByTime(const std::chrono::milliseconds& time_delta) {
     try {
        UpdateDogsCoordinatsByTime(time_delta);
        UpdateLootGenerationByTime(time_delta);
    } catch (const std::exception& e) {
            // Ловим исключения std::exception и добавляем информацию о функции
            throw std::runtime_error(std::string("Exception in UpdateSessionByTime: ") + e.what());
    } catch (...) {
        // Ловим все остальные исключения и добавляем информацию о функции
        throw std::runtime_error("Unknown exception in UpdateSessionByTime");
    }
}

void GameSession::UpdateDogsCoordinatsByTime(const std::chrono::milliseconds& time_delta_ms){

    try {

        int time_delta = static_cast<int>(time_delta_ms.count());

        //net::dispatch(*game_session_strand_, [self = shared_from_this(), &time_delta]() {

        for (auto& dog : dogs_) {
            Coordinates start = dog->GetCoordinate();
            Dimension start_x = static_cast<Dimension>(std::round(start.x));
            Dimension start_y = static_cast<Dimension>(std::round(start.y));

            Coordinates calc_finish = dog->GetCoordinateByTime(time_delta);
            Coordinates finish;
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

    } catch (const std::exception& e) {
           // Ловим исключения std::exception и добавляем информацию о функции
           throw std::runtime_error(std::string("Exception in UpdateDogsCoordinatsByTime: ") + e.what());
    } catch (...) {
       // Ловим все остальные исключения и добавляем информацию о функции
       throw std::runtime_error("Unknown exception in UpdateDogsCoordinatsByTime");
    }

}

void GameSession::UpdateLootGenerationByTime(const std::chrono::milliseconds& time_delta) {

    try{


        unsigned loot_count = lost_objects_.size();
        unsigned looter_count = dogs_.size();

        unsigned new_loot_count = loot_generator_.Generate(time_delta, loot_count, looter_count);

        for (unsigned i = 0; i < new_loot_count; ++i) {
            auto lost_object = std::make_shared<LostObject>();
            Point loot_coord = map_->GetRandomPointRoadMap();
            //std::cout << "loot x = " << loot_coord.x << " y = " << loot_coord.y << std::endl;
            lost_object->SetCoordinateByPoint(loot_coord);
            lost_object->SetType(GetRandomTypeLostObject());
            lost_objects_.insert(lost_object);
        }

    } catch (const std::exception& e) {
        // Ловим исключения std::exception и добавляем информацию о функции
        throw std::runtime_error(std::string("Exception in UpdateLootGenerationByTime: ") + e.what());
    } catch (...) {
        // Ловим все остальные исключения и добавляем информацию о функции
        throw std::runtime_error("Unknown exception in UpdateLootGenerationByTime");
    }


//    net::dispatch(*game_session_strand_, [self = shared_from_this(), &time_delta]() {
//            unsigned loot_count = self->lost_objects_.size();
//            unsigned looter_count = self->dogs_.size();

//            unsigned new_loot_count = self->loot_generator_.Generate(time_delta, loot_count, looter_count);

//            for (unsigned i = 0; i < new_loot_count; ++i) {
//                auto lost_object = std::make_shared<LostObject>();
//                Point loot_coord = self->map_->GetRandomPointRoadMap();
//                //std::cout << "loot x = " << loot_coord.x << " y = " << loot_coord.y << std::endl;
//                lost_object->SetCoordinateByPoint(loot_coord);
//                lost_object->SetType(self->GetRandomTypeLostObject());
//                self->lost_objects_.insert(lost_object);
//            }
//    });
    //net::dispatch(*api_strand, [self = shared_from_this(), &time_delta, &loot_count, &looter_count]() {
//        unsigned new_loot_count = loot_generator_.Generate(time_delta, loot_count, looter_count);

//        for (unsigned i = 0; i < new_loot_count; ++i) {
//            auto lost_object = std::make_shared<LostObject>();
//            lost_object->SetCoordinateByPoint(map_->GetRandomPointRoadMap());
//            lost_object->SetType(GetRandomTypeLostObject());
//            lost_objects_.insert(lost_object);
//        }
//        });
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

    loot_ticker_ = std::make_shared<time_tiker::Ticker>(
        *game_session_strand_,
        loot_generator_.GetPeriod(),
        [self_weak = weak_from_this()](std::chrono::milliseconds delta) {
            if(auto self = self_weak.lock()) {
                self->UpdateLootGenerationByTime(delta);
            }
        }
    );
    loot_ticker_->Start();


//    if(time_update_.count() != 0){
//        ticker_ = std::make_shared<time_tiker::Ticker>(
//            *game_session_strand_,
//            time_update_,
//            [self = shared_from_this()](std::chrono::milliseconds delta) {
//                self->UpdateSessionByTime(delta);
//            }
//        );
//        ticker_->Start();
//    }

//    loot_ticker_ = std::make_shared<time_tiker::Ticker>(
//        *game_session_strand_,
//        loot_generator_.GetPeriod(),
//        [self = shared_from_this()](std::chrono::milliseconds delta) {
//                self->UpdateLootGenerationByTime(delta);
//        }
//    );
//    loot_ticker_->Start();

    //std::cout << "loot = " << loot_generator_.GetPeriod().count() << std::endl;
}

const GameSession::Id &GameSession::GetId() const noexcept {
    return map_->GetId();
}

}

//    [this](std::chrono::milliseconds delta) {
//        UpdateSessionByTime(delta);
//    }
