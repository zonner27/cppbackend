#include "game_session.h"


namespace model {

void GameSession::AddDog(std::shared_ptr<Dog> dog) {
    dog->SetCoordinateByPoint(map_->GetStartPointRoadMap());
    dogs_.insert(dog);
}

void GameSession::SetDogsCoordinatsByTime(int time_delta){
    for (auto& dog : dogs_) {
        //std::cout << std::fixed << std::setprecision(4);
        Coordinates start = dog->GetCoordinate();
        //Dimension start_x = static_cast<Dimension>(start.x);
        //Dimension start_y = static_cast<Dimension>(start.y);
        Dimension start_x = static_cast<Dimension>(std::round(start.x));
        Dimension start_y = static_cast<Dimension>(std::round(start.y));

        //std::cout << "start x = " << start.x << " y = " << start.y << std::endl;
        Coordinates calc_finish = dog->GetCoordinateByTime(time_delta);
        //std::cout << "finish x = " << calc_finish.x << " y = " << calc_finish.y << std::endl;
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
                    //std::cout << "callc finish y = " << calc_finish.y << "  static_cast<double>(start_y) = " << static_cast<double>(start_y) << std::endl;
                    //std::cout << " static_cast<double>(start_y) = " << static_cast<double>(start_y) << " static_cast<double>(start_y) = " << static_cast<double>(start_y) << std::endl;
                    if (calc_finish.y - static_cast<double>(start_y) <= 0.4) {
                        finish.y = calc_finish.y;
                    } else {
                        finish.y = static_cast<double>(start_y) + 0.4;
                        dog->SetSpeed({0, 0});
                    }
                    //std::cout << " finish.y = " << finish.y << std::endl;
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

        //std::cout << "finish x = " << finish.x << " y = " << finish.y << std::endl;
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
