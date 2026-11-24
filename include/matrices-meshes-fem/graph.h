/**
 * @file graph.h
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a directed graph.
 */

#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#include <cstddef>
#include <vector>
#include <set>

/**
 * @class Graph
 * @brief A class to represent a directed graph built from node-to-neighbors mapping.
 *
 * Each node is connected to a set of neighboring nodes through directed edges.
 * An adjacency list is used, where each node maps to a set of node indices.
 */
class Graph
{
public:
    /**
     * @brief Constructs a directed graph using a node-to-neighbor mapping.
     *
     * @param node_to_neighbors A vector where each index represents a node,
     *        and the corresponding set contains indices of its neighboring nodes.
     *        Each neighbor represents a directed edge from the current node to that neighbor.
     */
    Graph(std::vector<std::set<std::size_t>> const &node_to_neighbors);

    /**
     * @brief Returns the adjacency list of the graph.
     *
     * @return A const reference to the adjacency list.
     */
    std::vector<std::set<std::size_t>> const &getAdjacency() const;

    /**
     * @brief Prints the adjacency list.
     */
    void print() const;

protected:
    /**
     * @brief Adjacency list representation of the graph.
     *
     * Each index corresponds to a node, and the set contains the indices
     * of nodes to which it has directed edges.
     */
    std::vector<std::set<std::size_t>> adj_;
};
#endif // GRAPH_H_INCLUDED
