#include <catch.hpp>

#include "PathSpace.hpp"


using namespace FSNG;

TEST_CASE("PathSpace Insert") {
    PathSpaceTE space = PathSpace{};
    Path const rootTestPath{"/test"};
    Path const rootTestPath2{"/test2"};
    Path const rootTestPath3{"/test3"};

    Path const rootTestTest2Path{"/test/test2"};

    SECTION("Basic") {
        REQUIRE(space.insert(rootTestPath, 5) == true);
        nlohmann::json json;
        json["test"] = {5};
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert(rootTestPath2, "hello") == true);
        json["test2"] = {"hello"};
        REQUIRE(space.toJSON() == json);

        REQUIRE(space.insert(rootTestPath, 234) == true);
        json["test"].push_back(234);
        REQUIRE(space.toJSON() == json);

        json["test"].push_back(2345);
        REQUIRE(space.toJSON() != json);
    }
}