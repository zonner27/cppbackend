#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>
#include "../src/serialization/model_serialization.h"
#include "../src/serialization/lost_object_serialization.h"
#include "../src/model/lost_object.h"

using namespace model;
using namespace std::literals;
namespace {

using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

struct Fixture {
    std::stringstream strm;
    OutputArchive output_archive{strm};
};

}  // namespace

SCENARIO_METHOD(Fixture, "Point serialization") {
    GIVEN("A point") {
        const geom::Point2D p{10, 20};
        WHEN("point is serialized") {
            output_archive << p;

            THEN("it is equal to point after serialization") {
                InputArchive input_archive{strm};
                geom::Point2D restored_point;
                input_archive >> restored_point;
                CHECK(p == restored_point);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Lost_object Serialization") {
    GIVEN("a lost_object") {
        const auto lost_object = [] {
            model::LostObject lost_object(2);
            lost_object.SetType(1);
            lost_object.SetCoordinate({42.2, 12.5});
            return lost_object;
        }();

        WHEN("lost_object is serialized") {
            {
                serialization::LostObjectRepr repr{lost_object};
                output_archive << repr;
            }

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::LostObjectRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore();

                CHECK(lost_object.GetId() == restored.GetId());
                CHECK(lost_object.GetType() == restored.GetType());
                CHECK(lost_object.GetCoordinate() == restored.GetCoordinate());
            }
        }
    }
}
