/**
 * @file weightedgraph.h
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a weighted directed graph.
 */

#ifndef WEIGHTEDGRAPH_H_INCLUDED
#define WEIGHTEDGRAPH_H_INCLUDED

#include "coord.h"
#include "graph.h"

#include <cstddef>
#include <map>
#include <set>
#include <utility>
#include <vector>

/**
 * @class WeightedGraph
 * @brief A class to represent a weighted directed graph.
 *
 * This class extends the Graph class by associating weights to edges.
 */
class WeightedGraph : public Graph {
public:
    /**
     * @brief Constructs a WeightedGraph from a node-to-neighbor mapping.
     *
     * The weights are computed based on the inverse squared node distance
     * between the nodes representing the edge.
     *
     * For each edge (v, w), the weight is defined as:
     *   \f$ w_{vw} = \frac{1}{||v - w||^2} \f$
     *
     * For reflexive edges (v, v), the weight is set to the **negative sum** of all outgoing
     * non-reflexive edge weights from that node:
     *   \f$ w_{vv} = -\sum_{w \ne v} w_{vw} \f$
     *
     * @param node_to_neighbors A vector where each index represents a node, and the set contains
     *        indices of its neighboring nodes (outgoing edges).
     * @param coords A vector of coordinates, where coords[i] corresponds to the position of node i.
     *
     * @throws std::invalid_argument If the number of coordinates does not match the number
     *         of nodes, if any two nodes share the same coordinates, or if a connected
     *         pair has an invalid distance.
     */
    WeightedGraph(std::vector<std::set<std::size_t>> const& node_to_neighbors,
                  std::vector<Coord> const& coords);

    /**
     * @brief Returns the edge-to-weight mapping.
     */
    std::map<std::pair<std::size_t, std::size_t>, double> const& getWeights() const;

    /**
     * @brief Prints the edge-to-weight mapping.
     *
     * Overrides Graph::print() so calls through a Graph reference use the weighted view.
     */
    void print() const override;

private:
    /**
     * @brief Edge-to-weight mapping.
     *
     * Each key is a pair (v, w) representing a directed edge,
     * and the value is the associated weight.
     */
    std::map<std::pair<std::size_t, std::size_t>, double> weights_;
};
#endif // WEIGHTEDGRAPH_H_INCLUDED
