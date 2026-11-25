#include <cmath>
#include <cstddef>
#include <set>
#include <stdexcept>
#include <string>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "matrices-meshes-fem/coord.h"
#include "matrices-meshes-fem/mesh.h"

namespace
{
    std::string dataPath(std::string const &filename)
    {
        return std::string(MMF_TEST_DATA_DIR) + "/" + filename;
    }
} // namespace

TEST_CASE("Mesh loads nodes and elements from files", "[mesh]")
{
    Mesh const mesh(dataPath("3x3_coords_2d.txt"), dataPath("3x3_elements_2d.txt"));

    REQUIRE(mesh.getNumNodes() == 16);
    REQUIRE(mesh.getNumElements() == 18);
    REQUIRE(mesh.getNodes().size() == 16);
    REQUIRE(mesh.getElements().size() == 18);

    REQUIRE(mesh.getNodes().front().x_ == Catch::Approx(1.0));
    REQUIRE(mesh.getNodes().front().y_ == Catch::Approx(0.333333));
    REQUIRE(mesh.getNodes().back().x_ == Catch::Approx(1.0));
    REQUIRE(mesh.getNodes().back().y_ == Catch::Approx(0.0));

    REQUIRE(mesh.getElements().front()[0] == 14);
    REQUIRE(mesh.getElements().front()[1] == 8);
    REQUIRE(mesh.getElements().front()[2] == 2);
    REQUIRE(mesh.getElements().back()[0] == 5);
    REQUIRE(mesh.getElements().back()[1] == 4);
    REQUIRE(mesh.getElements().back()[2] == 11);
}

TEST_CASE("Mesh constructor rejects missing files", "[mesh]")
{
    std::string const nodes = dataPath("3x3_coords_2d.txt");
    std::string const elements = dataPath("3x3_elements_2d.txt");
    std::string const missing = dataPath("does_not_exist.txt");

    SECTION("missing node file")
    {
        REQUIRE_THROWS_AS(Mesh(missing, elements), std::runtime_error);
    }

    SECTION("missing element file")
    {
        REQUIRE_THROWS_AS(Mesh(nodes, missing), std::runtime_error);
    }
}

TEST_CASE("Mesh node neighbors are symmetric and include self", "[mesh]")
{
    Mesh const mesh(dataPath("3x3_coords_2d.txt"), dataPath("3x3_elements_2d.txt"));
    auto const &neighbors = mesh.getNodeNeighbors();

    REQUIRE(neighbors.size() == mesh.getNumNodes());

    for (std::size_t n = 0; n < neighbors.size(); ++n)
    {
        REQUIRE(neighbors[n].count(n) == 1);
        for (std::size_t m : neighbors[n])
        {
            REQUIRE(neighbors[m].count(n) == 1);
        }
    }

    std::set<std::size_t> const expected{0, 3, 9, 13, 15};
    REQUIRE(neighbors[0] == expected);
}

TEST_CASE("Mesh point and triangle geometry helpers", "[mesh]")
{
    Mesh const mesh(dataPath("3x3_coords_2d.txt"), dataPath("3x3_elements_2d.txt"));
    Coord const a{0.0, 0.0};
    Coord const b{1.0, 0.0};
    Coord const c{0.0, 1.0};

    SECTION("pointInTriangle detects inside and outside points")
    {
        REQUIRE(mesh.pointInTriangle({0.25, 0.25}, a, b, c));
        REQUIRE_FALSE(mesh.pointInTriangle({1.0, 1.0}, a, b, c));
        REQUIRE(mesh.pointInTriangle({0.5, 0.0}, a, b, c));
        REQUIRE(mesh.pointInTriangle({0.25, 0.25}, a, c, b));
    }

    SECTION("pointToSegmentDistance clamps to segment bounds")
    {
        REQUIRE(mesh.pointToSegmentDistance({1.0, 1.0}, a, Coord{2.0, 0.0}) == Catch::Approx(1.0));
        REQUIRE(mesh.pointToSegmentDistance({-1.0, 0.0}, a, Coord{2.0, 0.0}) == Catch::Approx(1.0));
        REQUIRE(mesh.pointToSegmentDistance({3.0, 0.0}, a, Coord{2.0, 0.0}) == Catch::Approx(1.0));
        REQUIRE(mesh.pointToSegmentDistance({1.0, 0.0}, a, Coord{2.0, 0.0}) == Catch::Approx(0.0));
    }

    SECTION("pointToSegmentDistance handles degenerate segments")
    {
        REQUIRE(mesh.pointToSegmentDistance({3.0, 4.0}, a, a) == Catch::Approx(5.0));
        REQUIRE(mesh.pointToSegmentDistance(a, a, a) == Catch::Approx(0.0));
        REQUIRE(mesh.pointToTriangleDistance({-3.0, 4.0}, a, a, b) == Catch::Approx(5.0));
    }

    SECTION("pointToTriangleDistance measures distance to the boundary")
    {
        REQUIRE(mesh.pointToTriangleDistance({0.25, 0.25}, a, b, c) == Catch::Approx(0.0));
        REQUIRE(mesh.pointToTriangleDistance({0.5, -1.0}, a, b, c) == Catch::Approx(1.0));
        REQUIRE(mesh.pointToTriangleDistance({1.0, 1.0}, a, b, c) == Catch::Approx(std::sqrt(2.0) / 2.0));
    }

    SECTION("findElementShortestDistanceToPoint locates the closest element")
    {
        Coord const node0 = mesh.getNodes().front();
        auto const closest = mesh.findElementShortestDistanceToPoint(node0);

        REQUIRE(closest.second == Catch::Approx(0.0));
        REQUIRE(closest.first < mesh.getNumElements());

        auto const &element = mesh.getElements()[closest.first];
        REQUIRE((element[0] == 0 || element[1] == 0 || element[2] == 0));

        auto const far = mesh.findElementShortestDistanceToPoint({-1.0, -2.0});
        REQUIRE(far.first < mesh.getNumElements());
        REQUIRE(far.second > 0.0);
    }
}
