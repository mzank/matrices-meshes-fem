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
#include <set>
#include <stdexcept>
#include <utility>

//-----------------------------------------------------------------------------
WeightedGraph::WeightedGraph(std::vector<std::set<std::size_t>> const &node_to_neighbors,
                             std::vector<Coord> const &coords)
    : Graph(node_to_neighbors), weights_()
{
    if (coords.size() != adj_.size())
        throw std::invalid_argument("Number of coordinates must match number of nodes");

    std::set<std::pair<double, double>> positions;
    for (Coord const &coord : coords)
    {
        if (!positions.emplace(coord.x_, coord.y_).second)
            throw std::invalid_argument("Nodes must not be coincident");
    }

    std::ranges::for_each(std::views::iota(std::size_t{0}, adj_.size()),
                          [this, &coords](std::size_t const v)
                          {
                              double const sum_weights = std::accumulate(
                                  adj_[v].begin(), adj_[v].end(), 0.0,
                                  [this, &coords, v](double sum, std::size_t const w)
                                  {
                                      if (v == w)
                                          return sum;

                                      double const dx = coords[v].x_ - coords[w].x_;
                                      double const dy = coords[v].y_ - coords[w].y_;
                                      double const squared_distance = dx * dx + dy * dy;
                                      if (!(squared_distance > 0.0))
                                          throw std::invalid_argument("Invalid distance between connected nodes");

                                      double const weight = 1.0 / squared_distance;

                                      weights_[{v, w}] = weight;
                                      return sum + weight;
                                  });

                              weights_[{v, v}] = -sum_weights;
                          });
}

//-----------------------------------------------------------------------------
std::map<std::pair<std::size_t, std::size_t>, double> const &WeightedGraph::getWeights() const
{
    return weights_;
}

//-----------------------------------------------------------------------------
void WeightedGraph::print() const
{
    std::cout << "Weighted graph edges:\n";
    for (auto const &[edge, weight] : weights_)
    {
        std::cout << "(" << edge.first << ", " << edge.second << ") -> " << weight << "\n";
    }
}
