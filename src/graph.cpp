/**
 * @file graph.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a directed graph.
 */

#include "matrices-meshes-fem/graph.h"
#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
Graph::Graph(const vector<set<size_t>> &node_to_neighbors)
    : adj_(node_to_neighbors.size())
{
    for (size_t v = 0; v < node_to_neighbors.size(); ++v)
    {
        for (size_t w : node_to_neighbors[v])
        {
            adj_[v].insert(w);
        }
    }
}

//-----------------------------------------------------------------------------
const vector<set<size_t>> &Graph::getAdjacency() const
{
    return adj_;
}

//-----------------------------------------------------------------------------
void Graph::print() const
{
    cout << "Graph adjacency list:";
    for (size_t v = 0; v < adj_.size(); ++v)
    {
        cout << "\nNode " << v << ": ";
        for (size_t w : adj_[v])
        {
            cout << w << " ";
        }
    }
    cout << endl;
}
