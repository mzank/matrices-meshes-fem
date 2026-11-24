#include <catch2/catch_test_macros.hpp>

#include "matrices-meshes-fem/coord.h"

TEST_CASE("Coord stores 2D point components", "[coord]")
{
    SECTION("aggregate initialization sets both components")
    {
        Coord const p{1.5, -2.25};
        REQUIRE(p.x_ == 1.5);
        REQUIRE(p.y_ == -2.25);
    }

    SECTION("value initialization zeroes both components")
    {
        Coord const p{};
        REQUIRE(p.x_ == 0.0);
        REQUIRE(p.y_ == 0.0);
    }

    SECTION("components can be assigned independently")
    {
        Coord p{0.0, 0.0};
        p.x_ = 3.0;
        p.y_ = -4.0;
        REQUIRE(p.x_ == 3.0);
        REQUIRE(p.y_ == -4.0);
    }
}
