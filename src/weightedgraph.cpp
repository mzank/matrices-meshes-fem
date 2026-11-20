/**
 * @file weightedgraph.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a weighted directed graph.
 */

#include "matrices-meshes-fem/weightedgraph.h"
#include <cstddef>
#include <iostream>
#include <utility>

//-----------------------------------------------------------------------------
WeightedGraph::WeightedGraph(const std::vector<std::set<std::size_t>> &nodeToNeighbors,
                             const std::vector<Coord> &coords)
    : Graph(nodeToNeighbors), weights_()
{
    for (std::size_t v = 0; v < adj_.size(); ++v)
    {
        double sum_weights = 0.0;
        for (std::size_t w : adj_[v])
        {
            if (v == w)
                continue;

            double dx = coords[v].x_ - coords[w].x_;
            double dy = coords[v].y_ - coords[w].y_;

            double weight = 1.0 / (dx * dx + dy * dy);

            weights_[{v, w}] = weight;
            sum_weights += weight;
        }

        weights_[{v, v}] = -sum_weights;
    }
}

//-----------------------------------------------------------------------------
const std::map<std::pair<std::size_t, std::size_t>, double> &WeightedGraph::getWeights() const
{
    return weights_;
}

//-----------------------------------------------------------------------------
void WeightedGraph::print() const
{
    std::cout << "Weighted graph edges:\n";
    for (const auto &[edge, weight] : weights_)
    {
        std::cout << "(" << edge.first << ", " << edge.second << ") -> " << weight << "\n";
    }
}
