#include "item_gatherer_provider.h"

namespace collision_detector {

size_t collision_detector::ItemGathererDogProvider::ItemsCount() const {
    return items_.size();
}

Item ItemGathererDogProvider::GetItem(size_t idx) const {
    if (idx < items_.size()) {
        return items_[idx];
    }
    throw std::out_of_range("Index out of range");
}

void ItemGathererDogProvider::AddItem(Item item) {
    items_.push_back(std::move(item));
}

size_t ItemGathererDogProvider::GatherersCount() const {
    return gatherers_.size();
}

Gatherer ItemGathererDogProvider::GetGatherer(size_t idx) const {
    if (idx < gatherers_.size()) {
        return gatherers_[idx];
    }
    throw std::out_of_range("Index out of range");
}

void ItemGathererDogProvider::AddGatherer(Gatherer gatherer) {
    gatherers_.push_back(std::move(gatherer));
}


} // namespace collision_detector
