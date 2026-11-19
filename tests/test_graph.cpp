#include <cstddef>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "matrices-meshes-fem/graph.h"

TEST_CASE("Graph copies adjacency from node-to-neighbors mapping", "[graph]")
{
    const std::vector<std::set<std::size_t>> input{
        {1, 2},
        {0},
        {},
        {3}};

    const Graph graph(input);
    const auto &adjacency = graph.getAdjacency();

    REQUIRE(adjacency.size() == 4);
    REQUIRE((adjacency[0] == std::set<std::size_t>{1, 2}));
    REQUIRE((adjacency[1] == std::set<std::size_t>{0}));
    REQUIRE(adjacency[2].empty());
    REQUIRE((adjacency[3] == std::set<std::size_t>{3}));

    // Edges are directed: 0 -> 1 exists, but 1 -> 2 does not.
    REQUIRE(adjacency[0].count(1) == 1);
    REQUIRE(adjacency[1].count(2) == 0);
}

TEST_CASE("Graph owns its adjacency list", "[graph]")
{
    std::vector<std::set<std::size_t>> input{{1}, {}};
    const Graph graph(input);

    input[0].insert(2);
    input.emplace_back();

    REQUIRE(graph.getAdjacency().size() == 2);
    REQUIRE((graph.getAdjacency()[0] == std::set<std::size_t>{1}));
    REQUIRE(graph.getAdjacency()[1].empty());
}

TEST_CASE("Graph handles an empty node list", "[graph]")
{
    const std::vector<std::set<std::size_t>> empty;
    const Graph graph(empty);

    REQUIRE(graph.getAdjacency().empty());
}

TEST_CASE("Graph prints its adjacency list", "[graph]")
{
    const std::vector<std::set<std::size_t>> input{{1, 2}, {0}, {}, {3}};
    const Graph graph(input);

    std::ostringstream output;
    std::streambuf *old_buffer = std::cout.rdbuf(output.rdbuf());
    graph.print();
    std::cout.rdbuf(old_buffer);

    const std::string text = output.str();
    REQUIRE(text.find("Graph adjacency list:") != std::string::npos);
    REQUIRE(text.find("Node 0: 1 2") != std::string::npos);
    REQUIRE(text.find("Node 1: 0") != std::string::npos);
    REQUIRE(text.find("Node 2:") != std::string::npos);
    REQUIRE(text.find("Node 3: 3") != std::string::npos);
}
