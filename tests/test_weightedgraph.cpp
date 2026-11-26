#include <cstddef>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "capture_stdout.h"
#include "matrices-meshes-fem/coord.h"
#include "matrices-meshes-fem/weightedgraph.h"

namespace
{
    // Node 0 -> 1 has distance 1 (weight 1); node 0 -> 2 has distance 2 (weight 0.25).
    std::vector<std::set<std::size_t>> const &sampleNeighbors()
    {
        static std::vector<std::set<std::size_t>> const neighbors{
            {0, 1, 2},
            {0, 1},
            {0}};
        return neighbors;
    }

    std::vector<Coord> const &sampleCoords()
    {
        static std::vector<Coord> const coords{{0.0, 0.0}, {1.0, 0.0}, {0.0, 2.0}};
        return coords;
    }
} // namespace

TEST_CASE("WeightedGraph uses inverse squared distance and negative diagonal weights", "[weightedgraph]")
{
    WeightedGraph const graph(sampleNeighbors(), sampleCoords());
    auto const &weights = graph.getWeights();

    REQUIRE(weights.size() == 7);

    REQUIRE(weights.at({0, 1}) == Catch::Approx(1.0));
    REQUIRE(weights.at({0, 2}) == Catch::Approx(0.25));
    REQUIRE(weights.at({1, 0}) == Catch::Approx(1.0));
    REQUIRE(weights.at({2, 0}) == Catch::Approx(0.25));

    REQUIRE(weights.at({0, 0}) == Catch::Approx(-1.25));
    REQUIRE(weights.at({1, 1}) == Catch::Approx(-1.0));
    REQUIRE(weights.at({2, 2}) == Catch::Approx(-0.25));
}

TEST_CASE("WeightedGraph keeps directed edges and inherited adjacency", "[weightedgraph]")
{
    WeightedGraph const graph(sampleNeighbors(), sampleCoords());

    REQUIRE(graph.getWeights().count({1, 2}) == 0);
    REQUIRE(graph.getWeights().count({2, 1}) == 0);

    REQUIRE((graph.getAdjacency()[0] == std::set<std::size_t>{0, 1, 2}));
    REQUIRE((graph.getAdjacency()[1] == std::set<std::size_t>{0, 1}));
    REQUIRE((graph.getAdjacency()[2] == std::set<std::size_t>{0}));
}

TEST_CASE("WeightedGraph always inserts a diagonal weight", "[weightedgraph]")
{
    std::vector<std::set<std::size_t>> const neighbors{{}, {0}};
    std::vector<Coord> const coords{{0.0, 0.0}, {1.0, 0.0}};
    WeightedGraph const graph(neighbors, coords);

    REQUIRE(graph.getWeights().size() == 3);
    REQUIRE(graph.getWeights().at({0, 0}) == Catch::Approx(0.0));
    REQUIRE(graph.getWeights().at({1, 0}) == Catch::Approx(1.0));
    REQUIRE(graph.getWeights().at({1, 1}) == Catch::Approx(-1.0));
}

TEST_CASE("WeightedGraph handles an empty graph", "[weightedgraph]")
{
    std::vector<std::set<std::size_t>> const neighbors;
    std::vector<Coord> const coords;
    WeightedGraph const graph(neighbors, coords);

    REQUIRE(graph.getAdjacency().empty());
    REQUIRE(graph.getWeights().empty());
}

TEST_CASE("WeightedGraph rejects a coordinate count mismatch", "[weightedgraph]")
{
    std::vector<std::set<std::size_t>> const neighbors{{}, {0}};
    std::vector<Coord> const coords{{0.0, 0.0}};

    REQUIRE_THROWS_AS(WeightedGraph(neighbors, coords), std::invalid_argument);
}

TEST_CASE("WeightedGraph rejects coincident connected nodes", "[weightedgraph]")
{
    std::vector<std::set<std::size_t>> const neighbors{{0, 1}, {0}};
    std::vector<Coord> const coords{{1.0, 2.0}, {1.0, 2.0}};

    REQUIRE_THROWS_AS(WeightedGraph(neighbors, coords), std::invalid_argument);
}

TEST_CASE("WeightedGraph rejects coincident nodes without an edge", "[weightedgraph]")
{
    std::vector<std::set<std::size_t>> const neighbors{{}, {}};
    std::vector<Coord> const coords{{1.0, 2.0}, {1.0, 2.0}};

    REQUIRE_THROWS_AS(WeightedGraph(neighbors, coords), std::invalid_argument);
}

TEST_CASE("WeightedGraph rejects NaN distances", "[weightedgraph]")
{
    std::vector<std::set<std::size_t>> const neighbors{{0, 1}, {0}};
    std::vector<Coord> const coords{
        {0.0, 0.0}, {std::numeric_limits<double>::quiet_NaN(), 1.0}};

    REQUIRE_THROWS_AS(WeightedGraph(neighbors, coords), std::invalid_argument);
}

TEST_CASE("WeightedGraph prints its edge-to-weight mapping", "[weightedgraph]")
{
    WeightedGraph const graph(sampleNeighbors(), sampleCoords());

    std::ostringstream output;
    [[maybe_unused]] mmf_test::StdoutCapture const capture(output);
    graph.print();

    std::string const text = output.str();
    REQUIRE(text.find("Weighted graph edges:") != std::string::npos);
    REQUIRE(text.find("(0, 0) -> -1.25\n") != std::string::npos);
    REQUIRE(text.find("(0, 1) -> 1\n") != std::string::npos);
    REQUIRE(text.find("(0, 2) -> 0.25\n") != std::string::npos);
    REQUIRE(text.find("(2, 2) -> -0.25\n") != std::string::npos);
}

TEST_CASE("WeightedGraph print is polymorphic", "[weightedgraph]")
{
    static_assert(std::has_virtual_destructor_v<Graph>);

    WeightedGraph const graph(sampleNeighbors(), sampleCoords());
    Graph const &base = graph;

    std::ostringstream output;
    [[maybe_unused]] mmf_test::StdoutCapture const capture(output);
    base.print();

    std::string const text = output.str();
    REQUIRE(text.find("Weighted graph edges:") != std::string::npos);
    REQUIRE(text.find("Graph adjacency list:") == std::string::npos);
}
