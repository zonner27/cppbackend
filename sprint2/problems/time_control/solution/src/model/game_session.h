#pragma once

#include "model.h"
#include "dog.h"
#include <cmath>

namespace model {

class GameSession {
public:

    using Id = util::Tagged<std::string, Map>;

    GameSession(const Map* map) : map_{map} {}

    void AddDog(std::shared_ptr<Dog> dog);
    void SetDogsCoordinatsByTime(int time_delta);
    const std::string& GetMapName() const noexcept;
    const Map* GetMap() noexcept;
    const Id& GetId() const noexcept;
    const size_t GetDogsCount() const noexcept;
    std::unordered_set<std::shared_ptr<Dog>>& GetDogs() noexcept;

private:
    std::unordered_set<std::shared_ptr<Dog>> dogs_;
    const Map* map_;
};



} //namespace model
