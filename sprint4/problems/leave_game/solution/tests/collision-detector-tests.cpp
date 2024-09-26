#define _USE_MATH_DEFINES

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../src/events/collision_detector.h"

#include <sstream>

namespace Catch {
template<>
struct StringMaker<collision_detector::GatheringEvent> {
  static std::string convert(collision_detector::GatheringEvent const& value) {
      std::ostringstream tmp;
      tmp << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

      return tmp.str();
  }
};
}  // namespace Catch

namespace collision_detector {

class ConcreteItemGathererProvider : public ItemGathererProvider {
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

} //namespace collision_detecter

using namespace std::literals;

TEST_CASE("FindGatherEvents detects all events", "[FindGatherEvents]") {
    collision_detector::ConcreteItemGathererProvider provider;

    provider.AddItem({geom::Point2D{1, 1}, 0.5});
    provider.AddItem({geom::Point2D{2, 2}, 0.5});
    provider.AddItem({geom::Point2D{3, 3}, 0.5});
    provider.AddGatherer({geom::Point2D{0, 0}, geom::Point2D{4, 4}, 0.5});

    auto events = collision_detector::FindGatherEvents(provider);

    REQUIRE(events.size() == 3);
    CHECK(events[0].item_id == 0);
    CHECK(events[1].item_id == 1);
    CHECK(events[2].item_id == 2);
}

TEST_CASE("FindGatherEvents no events", "[FindGatherEvents]") {
    collision_detector::ConcreteItemGathererProvider provider;

    provider.AddItem({geom::Point2D{1, 1}, 0.1});
    provider.AddItem({geom::Point2D{2, 2}, 0.1});
    provider.AddItem({geom::Point2D{3, 3}, 0.1});
    provider.AddGatherer({geom::Point2D{0, 0}, geom::Point2D{4, 0}, 0.1});

    auto events = collision_detector::FindGatherEvents(provider);

    CHECK(events.empty());
}

TEST_CASE("FindGatherEvents Gather stop - no events", "[FindGatherEvents]") {
    collision_detector::ConcreteItemGathererProvider provider;

    provider.AddItem({geom::Point2D{2, 2}, 0.5});
    provider.AddGatherer({geom::Point2D{1, 1}, geom::Point2D{1, 1}, 0.5});

    auto events = collision_detector::FindGatherEvents(provider);

    CHECK(events.empty());
}

TEST_CASE("FindGatherEvents events are in chronological order", "[FindGatherEvents]") {
    collision_detector::ConcreteItemGathererProvider provider;

    provider.AddItem({geom::Point2D{1, 1}, 0.5});
    provider.AddItem({geom::Point2D{3, 3}, 0.5});
    provider.AddItem({geom::Point2D{2, 2}, 0.5});
    provider.AddGatherer({geom::Point2D{0, 0}, geom::Point2D{4, 4}, 0.5});

    auto events = collision_detector::FindGatherEvents(provider);

    REQUIRE(events.size() == 3);
    CHECK(events[0].time <= events[1].time);
    CHECK(events[1].time <= events[2].time);
}

TEST_CASE("FindGatherEvents correct event data", "[FindGatherEvents]") {
    collision_detector::ConcreteItemGathererProvider provider;

    provider.AddItem({geom::Point2D{2, 2}, 0.5});
    provider.AddGatherer({geom::Point2D{0, 0}, geom::Point2D{4, 4}, 0.5});

    auto events = collision_detector::FindGatherEvents(provider);

    REQUIRE(events.size() == 1);
    CHECK(events[0].item_id == 0);
    CHECK(events[0].gatherer_id == 0);
    CHECK_THAT(events[0].sq_distance, Catch::Matchers::WithinAbs(0, 1e-10));
    CHECK_THAT(events[0].time, Catch::Matchers::WithinAbs(0.5, 1e-10));
}

TEST_CASE("FindGatherEvents 2 gather", "[FindGatherEvents]") {
    collision_detector::ConcreteItemGathererProvider provider;

    provider.AddItem({geom::Point2D{1, 1}, 0.5});
    provider.AddItem({geom::Point2D{2, 2}, 0.5});
    provider.AddItem({geom::Point2D{3, 3}, 0.5});
    provider.AddGatherer({geom::Point2D{0, 0}, geom::Point2D{2, 2}, 0.5});
    provider.AddGatherer({geom::Point2D{1, 1}, geom::Point2D{3, 3}, 0.5});

    auto events = collision_detector::FindGatherEvents(provider);

    CHECK(events[0].gatherer_id == 1);
    CHECK(events[1].gatherer_id == 0);
    CHECK(events[2].gatherer_id == 1);
    CHECK(events[3].gatherer_id == 0);
    CHECK(events[4].gatherer_id == 1);


    CHECK(events[0].time <= events[1].time);
    CHECK(events[1].time <= events[2].time);
    CHECK(events[2].time <= events[3].time);
    CHECK(events[3].time <= events[4].time);
}

TEST_CASE("FindGatherEvents edge case test", "[FindGatherEvents]") {
    collision_detector::ConcreteItemGathererProvider provider;

    provider.AddItem({geom::Point2D{0, 0}, 0.5});
    provider.AddItem({geom::Point2D{4, 4}, 0.5});
    provider.AddGatherer({geom::Point2D{0, 0}, geom::Point2D{4, 4}, 0.5});

    auto events = collision_detector::FindGatherEvents(provider);

    REQUIRE(events.size() == 2);
    CHECK(events[0].item_id == 0);
    CHECK(events[1].item_id == 1);
    CHECK(events[0].gatherer_id == 0);
    CHECK(events[1].gatherer_id == 0);
    CHECK_THAT(events[0].time, Catch::Matchers::WithinAbs(0, 1e-9));
    CHECK_THAT(events[1].time, Catch::Matchers::WithinAbs(1, 1e-9));
}
