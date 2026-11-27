#include "capture_stdout.h"
#include "matrices-meshes-fem/coord.h"
#include "matrices-meshes-fem/mesh.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
std::string dataPath(std::string const& filename) {
    return std::string(MMF_TEST_DATA_DIR) + "/" + filename;
}

std::string captureOutput(std::function<void()> const& print) {
    std::ostringstream output;
    [[maybe_unused]] mmf_test::StdoutCapture const capture(output);
    print();
    return output.str();
}

std::string writeTempFile(std::string const& filename, std::string const& content) {
    std::filesystem::path const path = std::filesystem::temp_directory_path() / filename;
    std::ofstream out(path);
    out << content;
    return path.string();
}

Mesh smallMeshForMappings() {
    std::string const elements_path =
        writeTempFile("mmf_test_mapping_elements.txt", "3\n0 1 2\n1 2 3\n2 3 4\n");
    std::string const nodes_path =
        writeTempFile("mmf_test_mapping_nodes.txt", "5\n0 0\n1 0\n2 0\n3 0\n4 0\n");

    Mesh mesh(nodes_path, elements_path);
    std::filesystem::remove(elements_path);
    std::filesystem::remove(nodes_path);
    return mesh;
}
} // namespace

TEST_CASE("Mesh loads nodes and elements from files", "[mesh]") {
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

TEST_CASE("Mesh constructor rejects missing files", "[mesh]") {
    std::string const nodes = dataPath("3x3_coords_2d.txt");
    std::string const elements = dataPath("3x3_elements_2d.txt");
    std::string const missing = dataPath("does_not_exist.txt");

    SECTION("missing node file") {
        REQUIRE_THROWS_AS(Mesh(missing, elements), std::runtime_error);
    }

    SECTION("missing element file") {
        REQUIRE_THROWS_AS(Mesh(nodes, missing), std::runtime_error);
    }
}

TEST_CASE("Mesh node neighbors are symmetric and include self", "[mesh]") {
    Mesh const mesh(dataPath("3x3_coords_2d.txt"), dataPath("3x3_elements_2d.txt"));
    auto const& neighbors = mesh.getNodeNeighbors();

    REQUIRE(neighbors.size() == mesh.getNumNodes());

    for (std::size_t n = 0; n < neighbors.size(); ++n) {
        REQUIRE(neighbors[n].count(n) == 1);
        for (std::size_t m : neighbors[n]) {
            REQUIRE(neighbors[m].count(n) == 1);
        }
    }

    std::set<std::size_t> const expected{0, 3, 9, 13, 15};
    REQUIRE(neighbors[0] == expected);
}

TEST_CASE("Mesh mapping builders produce expected mappings", "[mesh]") {
    Mesh const mesh = smallMeshForMappings();

    auto const& node_to_elements = mesh.getNodeToElements();
    REQUIRE(node_to_elements.size() == 5);
    REQUIRE((node_to_elements[0] == std::vector<std::size_t>{0}));
    REQUIRE((node_to_elements[1] == std::vector<std::size_t>{0, 1}));
    REQUIRE((node_to_elements[2] == std::vector<std::size_t>{0, 1, 2}));
    REQUIRE((node_to_elements[3] == std::vector<std::size_t>{1, 2}));
    REQUIRE((node_to_elements[4] == std::vector<std::size_t>{2}));

    std::map<std::pair<std::size_t, std::size_t>, std::vector<std::size_t>> const expected_edges{
        {{0, 1}, {0}},    {{0, 2}, {0}}, {{1, 2}, {0, 1}}, {{1, 3}, {1}},
        {{2, 3}, {1, 2}}, {{2, 4}, {2}}, {{3, 4}, {2}}};
    REQUIRE(mesh.getEdgeToElements() == expected_edges);

    auto const& element_neighbors = mesh.getElementNeighbors();
    REQUIRE(element_neighbors.size() == 3);
    REQUIRE((element_neighbors[0] == std::set<std::size_t>{1}));
    REQUIRE((element_neighbors[1] == std::set<std::size_t>{0, 2}));
    REQUIRE((element_neighbors[2] == std::set<std::size_t>{1}));
}

TEST_CASE("Mesh mapping builders are idempotent", "[mesh]") {
    Mesh mesh(dataPath("3x3_coords_2d.txt"), dataPath("3x3_elements_2d.txt"));

    auto const node_to_elements_before = mesh.getNodeToElements();
    auto const edge_to_elements_before = mesh.getEdgeToElements();
    auto const element_neighbors_before = mesh.getElementNeighbors();
    auto const node_neighbors_before = mesh.getNodeNeighbors();
    auto const node_to_elements_text_before =
        captureOutput([&mesh] { mesh.printNodeToElements(); });
    auto const edge_to_elements_text_before =
        captureOutput([&mesh] { mesh.printEdgeToElements(); });
    auto const element_neighbors_text_before =
        captureOutput([&mesh] { mesh.printElementNeighbors(); });

    mesh.nodeToElements();
    mesh.nodeToElements();
    mesh.edgeToElements();
    mesh.edgeToElements();
    mesh.elementNeighbors();
    mesh.elementNeighbors();
    mesh.nodeNeighbors();
    mesh.nodeNeighbors();

    REQUIRE(mesh.getNodeToElements() == node_to_elements_before);
    REQUIRE(mesh.getEdgeToElements() == edge_to_elements_before);
    REQUIRE(mesh.getElementNeighbors() == element_neighbors_before);
    REQUIRE(mesh.getNodeNeighbors() == node_neighbors_before);
    REQUIRE(captureOutput([&mesh] { mesh.printNodeToElements(); }) == node_to_elements_text_before);
    REQUIRE(captureOutput([&mesh] { mesh.printEdgeToElements(); }) == edge_to_elements_text_before);
    REQUIRE(captureOutput([&mesh] { mesh.printElementNeighbors(); }) ==
            element_neighbors_text_before);
}

TEST_CASE("Mesh file readers rebuild derived mappings and are transactional", "[mesh]") {
    std::string const small_elements =
        writeTempFile("mmf_test_small_elements.txt", "3\n0 1 2\n1 2 3\n2 3 4\n");
    std::string const small_nodes =
        writeTempFile("mmf_test_small_nodes.txt", "5\n0 0\n1 0\n2 0\n3 0\n4 0\n");
    std::string const too_few_nodes = writeTempFile("mmf_test_too_few_nodes.txt", "2\n0 0\n1 0\n");

    Mesh mesh(dataPath("3x3_coords_2d.txt"), dataPath("3x3_elements_2d.txt"));
    auto const original_nodes = mesh.getNumNodes();
    auto const original_elements = mesh.getNumElements();
    auto const original_neighbors = mesh.getNodeNeighbors();

    REQUIRE_FALSE(mesh.readElements(dataPath("does_not_exist.txt")));
    REQUIRE(mesh.getNumElements() == original_elements);
    REQUIRE(mesh.getNodeNeighbors() == original_neighbors);

    REQUIRE(mesh.readElements(small_elements));
    REQUIRE(mesh.getNumElements() == 3);
    REQUIRE((mesh.getNodeNeighbors()[0] == std::set<std::size_t>{0, 1, 2}));
    REQUIRE((mesh.getNodeNeighbors()[4] == std::set<std::size_t>{2, 3, 4}));

    REQUIRE_FALSE(mesh.readNodes(dataPath("does_not_exist.txt")));
    REQUIRE(mesh.getNumNodes() == original_nodes);

    REQUIRE_FALSE(mesh.readNodes(too_few_nodes));
    REQUIRE(mesh.getNumNodes() == original_nodes);

    REQUIRE(mesh.readNodes(small_nodes));
    REQUIRE(mesh.getNumNodes() == 5);
    REQUIRE(mesh.getNodes().size() == 5);
    REQUIRE(mesh.getNodeNeighbors().size() == 5);

    std::filesystem::remove(small_elements);
    std::filesystem::remove(small_nodes);
    std::filesystem::remove(too_few_nodes);
}

TEST_CASE("Mesh point and triangle geometry helpers", "[mesh]") {
    Mesh const mesh(dataPath("3x3_coords_2d.txt"), dataPath("3x3_elements_2d.txt"));
    Coord const a{0.0, 0.0};
    Coord const b{1.0, 0.0};
    Coord const c{0.0, 1.0};

    SECTION("pointInTriangle detects inside and outside points") {
        REQUIRE(mesh.pointInTriangle({0.25, 0.25}, a, b, c));
        REQUIRE_FALSE(mesh.pointInTriangle({1.0, 1.0}, a, b, c));
        REQUIRE(mesh.pointInTriangle({0.5, 0.0}, a, b, c));
        REQUIRE(mesh.pointInTriangle({0.25, 0.25}, a, c, b));
    }

    SECTION("pointToSegmentDistance clamps to segment bounds") {
        REQUIRE(mesh.pointToSegmentDistance({1.0, 1.0}, a, Coord{2.0, 0.0}) == Catch::Approx(1.0));
        REQUIRE(mesh.pointToSegmentDistance({-1.0, 0.0}, a, Coord{2.0, 0.0}) == Catch::Approx(1.0));
        REQUIRE(mesh.pointToSegmentDistance({3.0, 0.0}, a, Coord{2.0, 0.0}) == Catch::Approx(1.0));
        REQUIRE(mesh.pointToSegmentDistance({1.0, 0.0}, a, Coord{2.0, 0.0}) == Catch::Approx(0.0));
    }

    SECTION("pointToSegmentDistance handles degenerate segments") {
        REQUIRE(mesh.pointToSegmentDistance({3.0, 4.0}, a, a) == Catch::Approx(5.0));
        REQUIRE(mesh.pointToSegmentDistance(a, a, a) == Catch::Approx(0.0));
        REQUIRE(mesh.pointToTriangleDistance({-3.0, 4.0}, a, a, b) == Catch::Approx(5.0));
    }

    SECTION("pointToTriangleDistance measures distance to the boundary") {
        REQUIRE(mesh.pointToTriangleDistance({0.25, 0.25}, a, b, c) == Catch::Approx(0.0));
        REQUIRE(mesh.pointToTriangleDistance({0.5, -1.0}, a, b, c) == Catch::Approx(1.0));
        REQUIRE(mesh.pointToTriangleDistance({1.0, 1.0}, a, b, c) ==
                Catch::Approx(std::sqrt(2.0) / 2.0));
    }

    SECTION("findElementShortestDistanceToPoint locates the closest element") {
        Coord const node0 = mesh.getNodes().front();
        auto const closest = mesh.findElementShortestDistanceToPoint(node0);

        REQUIRE(closest.second == Catch::Approx(0.0));
        REQUIRE(closest.first < mesh.getNumElements());

        auto const& element = mesh.getElements()[closest.first];
        REQUIRE((element[0] == 0 || element[1] == 0 || element[2] == 0));

        auto const far = mesh.findElementShortestDistanceToPoint({-1.0, -2.0});
        REQUIRE(far.first < mesh.getNumElements());
        REQUIRE(far.second > 0.0);
    }
}
