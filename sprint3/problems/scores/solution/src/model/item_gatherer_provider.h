#pragma once
#include "../events/collision_detector.h"

namespace collision_detector {


class ItemGathererDogProvider : public ItemGathererProvider {
public:

    size_t ItemsCount() const override {
        return items_.size();
    }

    Item GetItem(size_t idx) const override {
        if (idx < items_.size()) {
            return items_[idx];
        }
        throw std::out_of_range("Index out of range");
    }

    void AddItem(Item item) {
        items_.push_back(std::move(item));
    }

    size_t GatherersCount() const override {
        return gatherers_.size();
    }

    Gatherer GetGatherer(size_t idx) const override {
        if (idx < gatherers_.size()) {
            return gatherers_[idx];
        }
        throw std::out_of_range("Index out of range");
    }

    void AddGatherer(Gatherer gatherer) {
        gatherers_.push_back(std::move(gatherer));
    }

private:
    std::vector<Item> items_;
    std::vector<Gatherer> gatherers_;
};


} // namespace collision_detector
