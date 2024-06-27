#include "game_session.h"


namespace model {

void GameSession::AddDog(std::shared_ptr<Dog> dog, bool randomize_spawn_points) {
    if (randomize_spawn_points) {
        dog->SetCoordinateByPoint(map_->GetRandomPointRoadMap());
        //std::cout << "Dogs coord x = " << dog->GetCoordinate().x << " y = " << dog->GetCoordinate().y << std::endl;
    } else {
       dog->SetCoordinateByPoint(map_->GetStartPointRoadMap());
       //std::cout << "Dogs coord x = " << dog->GetCoordinate().x << " y = " << dog->GetCoordinate().y << std::endl;
    }

    dogs_.insert(dog);
}

void GameSession::SetDogsCoordinatsByTime(int time_delta){
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
                    if (calc_finish.x - road_x_end <= 0.4) {
                        finish.x = calc_finish.x;
                    } else {
                        finish.x = road_x_end + 0.4;
                        dog->SetSpeed({0, 0});
                    }
                }
                if (direction == constants::Direction::WEST) {
                    double road_x_begin = static_cast<double>(road_cur->GetStart().x);
                    if (road_x_begin - calc_finish.x <= 0.4) {
                        finish.x = calc_finish.x;
                    } else {
                        finish.x = road_x_begin - 0.4;
                        dog->SetSpeed({0, 0});
                    }
                }
            } else {
                if (direction == constants::Direction::EAST) {
                    if (calc_finish.x - static_cast<double>(start_x) <= 0.4) {
                        finish.x = calc_finish.x;
                    } else {
                        finish.x = static_cast<double>(start_x) + 0.4;
                        dog->SetSpeed({0, 0});
                    }
                }
                if (direction == constants::Direction::WEST) {
                    if (static_cast<double>(start_x) - calc_finish.x <= 0.4) {
                        finish.x = calc_finish.x;
                    } else {
                        finish.x = static_cast<double>(start_x) - 0.4;
                        dog->SetSpeed({0, 0});
                    }
                }
            }
        } else if (direction == constants::Direction::NORTH || direction == constants::Direction::SOUTH) {
            finish.x = start.x;
            if (const Road* road_cur = map_->GetVerRoad(start_x, start_y)) {
                if (direction == constants::Direction::SOUTH) {
                    double road_y_end = static_cast<double>(road_cur->GetEnd().y);
                    if (calc_finish.y - road_y_end <= 0.4) {
                        finish.y = calc_finish.y;
                    } else {
                        finish.y = road_y_end + 0.4;
                        dog->SetSpeed({0, 0});
                    }
                }
                if (direction == constants::Direction::NORTH) {
                    double road_y_begin = static_cast<double>(road_cur->GetStart().y);
                    if (road_y_begin - calc_finish.y <= 0.4) {
                        finish.y = calc_finish.y;
                    } else {
                        finish.y = road_y_begin - 0.4;
                        dog->SetSpeed({0, 0});
                    }
                }
            } else {
                if (direction == constants::Direction::SOUTH) {
                    if (calc_finish.y - static_cast<double>(start_y) <= 0.4) {
                        finish.y = calc_finish.y;
                    } else {
                        finish.y = static_cast<double>(start_y) + 0.4;
                        dog->SetSpeed({0, 0});
                    }
                }
                if (direction == constants::Direction::NORTH) {
                    if (static_cast<double>(start_y) - calc_finish.y <= 0.4) {
                        finish.y = calc_finish.y;
                    } else {
                        finish.y = static_cast<double>(start_y) - 0.4;
                        dog->SetSpeed({0, 0});
                    }
                }
            }
        }
        dog->SetCoordinate(finish);
    }
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

const GameSession::Id &GameSession::GetId() const noexcept {
    return map_->GetId();
}

}
