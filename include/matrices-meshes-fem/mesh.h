/**
 * @file mesh.h
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a 2D triangular mesh consisting of nodes and elements.
 */

#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

#include <cstddef>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <utility>
#include "coord.h"

/**
 * @class Mesh
 * @brief Represents a 2D triangular mesh consisting of nodes and elements.
 *
 * This class provides functionality for reading mesh data from files, mappings
 * between mesh components (nodes, elements, edges), and calculate
 * point-to-triangle distance.
 */
class Mesh
{
public:
    /**
     * @brief Constructs a Mesh object by reading node and element data from files.
     *
     * This constructor initializes the mesh by reading the specified node and element files.
     * It generates the following mappings:
     * - Nodes to elements: Nodes of a triangular element.
     * - Edges to elements: Edges of a triangular element.
     * - Element neighbors: Two elements sharing an edge.
     * - Nodes to nodes: For each node, its neighbors including itself.
     *
     * If the node or element files cannot be read successfully, a `std::runtime_error` is thrown.
     *
     * @param node_file Path to the node file.
     * @param element_file Path to the element file.
     *
     * @throws std::runtime_error if reading from either the node or element file fails.
     */
    Mesh(const std::string &node_file, const std::string &element_file);

    /**
     * @brief Returns the list of node coordinates.
     * @return Constant reference to a vector of coordinates Coord.
     */
    const std::vector<Coord> &getNodes() const;

    /**
     * @brief Returns the number of nodes of the mesh.
     * @return Number of nodes.
     */
    std::size_t getNumNodes() const;

    /**
     * @brief Returns the list of triangular elements.
     * Each element is represented as an array of 3 node indices.
     * @return Constant reference to a vector of triangular elements.
     */
    const std::vector<std::array<std::size_t, 3>> &getElements() const;

    /**
     * @brief Returns the number of elements of the mesh.
     * @return Number of elements.
     */
    std::size_t getNumElements() const;

    /**
     * @brief Returns the neighboring nodes for each node. The indices of the neighboring nodes are
     * in sorted order.
     * @return Constant reference to a vector of sets, each set containing indices of neighboring nodes.
     */
    const std::vector<std::set<std::size_t>> &getNodeNeighbors() const;

    /**
     * @brief Reads node coordinates from a file.
     * @param filename Path to the node file.
     * @return True if successful, false otherwise.
     */
    bool readNodes(const std::string &filename);

    /**
     * @brief Reads element connectivity from a file.
     * @param filename Path to the element file.
     * @return True if successful, false otherwise.
     */
    bool readElements(const std::string &filename);

    /**
     * @brief Builds a mapping from nodes to the elements they belong to.
     */
    void nodeToElements();

    /**
     * @brief Prints number of nodes and number of elements of the mesh.
     */
    void printSummary() const;

    /**
     * @brief Prints the node-to-elements mapping.
     */
    void printNodeToElements() const;

    /**
     * @brief Builds a mapping from edges to the elements they are part of.
     */
    void edgeToElements();

    /**
     * @brief Prints the edge-to-elements mapping.
     */
    void printEdgeToElements() const;

    /**
     * @brief Builds a mapping from elements to neighboring elements.
     * Elements are neighboring when sharing an edge.
     */
    void elementNeighbors();

    /**
     * @brief Prints the element-to-neighboring elements mapping.
     */
    void printElementNeighbors() const;

    /**
     * @brief Build a mapping from nodes to neighboring nodes, where each node
     * is a neighbor of itself.
     */
    void nodeNeighbors();

    /**
     * @brief Prints the node-to-nodes mapping.
     */
    void printNodeNeighbors() const;

    /**
     * @brief Checks if a point lies inside a triangle defined by three coordinates Coord.
     *
     * The same-side technique is used, i.e., it checks on which side the point p lies
     * for every edge. This is realized by calculating cross products. An alternative approach
     * is the Barycentric technique, computing Barycentric coordinates.
     *
     * @param p The point to check.
     * @param a Coordinates Coord of the first node of the triangle.
     * @param b Coordinates Coord of the second node of the triangle.
     * @param c Coordinates Coord of the third node of the triangle.
     * @return True if the point is inside the triangle, false otherwise.
     */
    bool pointInTriangle(const Coord &p, const Coord &a, const Coord &b, const Coord &c) const;

    /**
     * @brief Computes the shortest distance from a point to a line segment.
     *
     * It works by projecting the point onto the straight line
     * defined by the segment, then clamping the projection to the
     * segment bounds if it lies outside.
     *
     * @param p The given point.
     * @param a Coordinates Coord of the first endpoint of the segment.
     * @param b Coordinates Coord of the second endpoint of the segment.
     * @return Distance from the point to the segment.
     */
    double pointToSegmentDistance(const Coord &p, const Coord &a, const Coord &b) const;

    /**
     * @brief Computes the shortest distance from a point to a triangle.
     * @param p The point.
     * @param a Coordinates Coord of the first node of the triangle.
     * @param b Coordinates Coord of the second node of the triangle.
     * @param c Coordinates Coord of the third node of the triangle.
     * @return Distance from the point to the triangle.
     */
    double pointToTriangleDistance(const Coord &p, const Coord &a, const Coord &b, const Coord &c) const;

    /**
     * @brief Finds the triangular element with the shortest distance to a given point.
     * @param p The point.
     * @return A pair containing the index of the closest element and the distance.
     */
    std::pair<std::size_t, double> findElementShortestDistanceToPoint(const Coord &p) const;

private:
    std::vector<Coord> nodes_;                                                 ///< List of node coordinates.
    std::size_t num_nodes_;                                                    ///< Number of nodes.
    std::vector<std::array<std::size_t, 3>> elements_;                         ///< List of triangular elements.
    std::size_t num_elements_;                                                 ///< Number of elements.
    std::vector<std::vector<std::size_t>> node_to_elements_;                   ///< Node-to-Element mapping. Node is part of the triangular element.
    std::map<std::pair<int, int>, std::vector<std::size_t>> edge_to_elements_; ///< Edge-to-element mapping. Edge is part or the triangular element.
    std::vector<std::set<std::size_t>> element_neighbors_;                     ///< Neighboring elements for each element, sharing an edge.
    std::vector<std::set<std::size_t>> node_neighbors_;                        ///< Neighboring nodes for each node, including itself.
};
#endif // MESH_H_INCLUDED
