/**
 * @file weightedgraph.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a weighted directed graph.
 */

#include "matrices-meshes-fem/weightedgraph.h"
#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
WeightedGraph::WeightedGraph(const vector<set<size_t>> &nodeToNeighbors,
                             const vector<Coord> &coords)
    : Graph(nodeToNeighbors), weights_()
{
    for (size_t v = 0; v < adj_.size(); ++v)
    {
        double sum_weights = 0.0;
        for (size_t w : adj_[v])
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
const map<pair<size_t, size_t>, double> &WeightedGraph::getWeights() const
{
    return weights_;
}

//-----------------------------------------------------------------------------
void WeightedGraph::print() const
{
    cout << "Weighted graph edges:\n";
    for (const auto &[edge, weight] : weights_)
    {
        cout << "(" << edge.first << ", " << edge.second << ") -> " << weight << "\n";
    }
}
