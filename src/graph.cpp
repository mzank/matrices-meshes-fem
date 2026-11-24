/**
 * @file graph.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a directed graph.
 */

#include "matrices-meshes-fem/graph.h"
#include <cstddef>
#include <iostream>

//-----------------------------------------------------------------------------
Graph::Graph(const std::vector<std::set<std::size_t>> &node_to_neighbors)
    : adj_(node_to_neighbors)
{
}

//-----------------------------------------------------------------------------
const std::vector<std::set<std::size_t>> &Graph::getAdjacency() const
{
    return adj_;
}

//-----------------------------------------------------------------------------
void Graph::print() const
{
    std::cout << "Graph adjacency list:";
    for (std::size_t v = 0; v < adj_.size(); ++v)
    {
        std::cout << "\nNode " << v << ": ";
        for (std::size_t w : adj_[v])
        {
            std::cout << w << " ";
        }
    }
    std::cout << "\n";
}
