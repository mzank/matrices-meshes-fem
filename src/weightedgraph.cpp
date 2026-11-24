/**
 * @file weightedgraph.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a weighted directed graph.
 */

#include "matrices-meshes-fem/weightedgraph.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <ranges>
#include <utility>

//-----------------------------------------------------------------------------
WeightedGraph::WeightedGraph(const std::vector<std::set<std::size_t>> &node_to_neighbors,
                             const std::vector<Coord> &coords)
    : Graph(node_to_neighbors), weights_()
{
    std::ranges::for_each(std::views::iota(std::size_t{0}, adj_.size()),
                          [this, &coords](std::size_t v)
                          {
                              const double sum_weights = std::accumulate(
                                  adj_[v].begin(), adj_[v].end(), 0.0,
                                  [this, &coords, v](double sum, std::size_t w)
                                  {
                                      if (v == w)
                                          return sum;

                                      const double dx = coords[v].x_ - coords[w].x_;
                                      const double dy = coords[v].y_ - coords[w].y_;
                                      const double weight = 1.0 / (dx * dx + dy * dy);

                                      weights_[{v, w}] = weight;
                                      return sum + weight;
                                  });

                              weights_[{v, v}] = -sum_weights;
                          });
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
