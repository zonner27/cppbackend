#pragma once
#include "../events/collision_detector.h"

namespace collision_detector {

class ItemGathererDogProvider : public ItemGathererProvider {
public:

    size_t ItemsCount() const override;
    Item GetItem(size_t idx) const override;
    void AddItem(Item item);
    size_t GatherersCount() const override;
    Gatherer GetGatherer(size_t idx) const override;
    void AddGatherer(Gatherer gatherer);

private:
    std::vector<Item> items_;
    std::vector<Gatherer> gatherers_;
};


} // namespace collision_detector
