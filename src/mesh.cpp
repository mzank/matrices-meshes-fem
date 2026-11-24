/**
 * @file mesh.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a 2D triangular mesh consisting of nodes and elements.
 */

#include "matrices-meshes-fem/mesh.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>

//------------------------------------------------------------------------------
Mesh::Mesh(const std::string &node_file, const std::string &element_file)
    : nodes_(), num_nodes_(0), elements_(), num_elements_(0), node_to_elements_(), edge_to_elements_(), element_neighbors_(), node_neighbors_()
{
    if (!readNodes(node_file))
        throw std::runtime_error("Mesh constructor: Failed to read node file '" + node_file + "'");

    if (!readElements(element_file))
        throw std::runtime_error("Mesh constructor: Failed to read element file '" + element_file + "'");

    nodeToElements();
    edgeToElements();
    element_neighbors_.resize(num_elements_);
    elementNeighbors();
    node_neighbors_.resize(num_nodes_);
    nodeNeighbors();
}

//------------------------------------------------------------------------------
const std::vector<Coord> &Mesh::getNodes() const
{
    return nodes_;
}

//------------------------------------------------------------------------------
std::size_t Mesh::getNumNodes() const
{
    return num_nodes_;
}

//------------------------------------------------------------------------------
const std::vector<std::array<std::size_t, 3>> &Mesh::getElements() const
{
    return elements_;
}

//------------------------------------------------------------------------------
std::size_t Mesh::getNumElements() const
{
    return num_elements_;
}

//------------------------------------------------------------------------------
const std::vector<std::set<std::size_t>> &Mesh::getNodeNeighbors() const
{
    return node_neighbors_;
}

//------------------------------------------------------------------------------
bool Mesh::readNodes(const std::string &node_file)
{
    std::ifstream coords_in(node_file);
    if (!coords_in)
    {
        std::cerr << "Failed to open coords file: " << node_file << "\n";
        return false;
    }

    coords_in >> num_nodes_;
    if (!coords_in)
    {
        std::cerr << "Error reading number of nodes_.\n";
        return false;
    }

    nodes_.resize(num_nodes_);
    for (std::size_t i = 0; i < num_nodes_; ++i)
    {
        coords_in >> nodes_[i].x_ >> nodes_[i].y_;
        if (!coords_in)
        {
            std::cerr << "Error reading node " << i << ".\n";
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
bool Mesh::readElements(const std::string &element_file)
{
    std::ifstream elem_in(element_file);
    if (!elem_in)
    {
        std::cerr << "Failed to open elements file: " << element_file << "\n";
        return false;
    }

    elem_in >> num_elements_;
    if (!elem_in)
    {
        std::cerr << "Error reading number of elements.\n";
        return false;
    }

    elements_.resize(num_elements_);
    for (std::size_t i = 0; i < num_elements_; ++i)
    {
        elem_in >> elements_[i][0] >> elements_[i][1] >> elements_[i][2];
        if (!elem_in)
        {
            std::cerr << "Error reading element " << i << ".\n";
            return false;
        }
    }
    return true;
}

//------------------------------------------------------------------------------
void Mesh::printSummary() const
{
    std::cout << "Mesh Summary:\n";
    std::cout << "Number of nodes: " << nodes_.size() << "\n";
    std::cout << "Number of elements: " << elements_.size() << "\n";
}

//------------------------------------------------------------------------------
void Mesh::nodeToElements()
{
    node_to_elements_.resize(num_nodes_);
    for (std::size_t e = 0; e < num_elements_; ++e)
    {
        for (std::size_t n = 0; n < 3; n++)
        {
            std::size_t node = elements_[e][n];
            node_to_elements_[node].push_back(e);
        }
    }
}

//------------------------------------------------------------------------------
void Mesh::printNodeToElements() const
{
    std::cout << "Node -> Elements:\n";
    for (std::size_t n = 0; n < num_nodes_; ++n)
    {
        std::cout << "Node " << n << ": ";
        for (auto e : node_to_elements_[n])
            std::cout << e << " ";
        std::cout << "\n";
    }
}

//------------------------------------------------------------------------------
void Mesh::edgeToElements()
{
    for (std::size_t e = 0; e < num_elements_; ++e)
    {
        auto &nodes_el = elements_[e];
        // Edges: (v0,v1), (v1,v2), (v2,v0)
        for (std::size_t i = 0; i < 3; i++)
        {
            std::size_t a = nodes_el[i];
            std::size_t b = nodes_el[(i + 1) % 3];
            // edges are stored such that smaller node index is the left edge index as in DG
            if (a > b)
                std::swap(a, b);
            edge_to_elements_[{a, b}].push_back(e);
        }
    }
}

//------------------------------------------------------------------------------
void Mesh::printEdgeToElements() const
{
    std::cout << "\nEdge-to-elements mapping:";
    for (const auto &entry : edge_to_elements_)
    {
        const auto &edge = entry.first;
        const auto &elements = entry.second;

        std::cout << "\nEdge (" << edge.first << ", " << edge.second << "): [";
        for (std::size_t i = 0; i < elements.size(); ++i)
        {
            std::cout << elements[i];
            if (i + 1 < elements.size())
                std::cout << ", ";
        }
        std::cout << "]";
    }
    std::cout << "\n";
}

//------------------------------------------------------------------------------
void Mesh::elementNeighbors()
{
    for (auto &[edge, elems] : edge_to_elements_)
    {
        // if edge is part of two elements
        if (elems.size() == 2)
        {
            std::size_t e1 = elems[0], e2 = elems[1];
            element_neighbors_[e1].insert(e2);
            element_neighbors_[e2].insert(e1);
        }
        // if elems.size() == 1:
        // edge is part of only one element, i.e., a boundary edge
    }
}

//------------------------------------------------------------------------------
void Mesh::printElementNeighbors() const
{
    std::cout << "\nElement -> Neighbors:";
    for (std::size_t e = 0; e < num_elements_; ++e)
    {
        std::cout << "\nElement " << e << ": ";
        for (auto n : element_neighbors_[e])
            std::cout << n << " ";
    }
    std::cout << "\n";
}

//------------------------------------------------------------------------------
void Mesh::nodeNeighbors()
{
    for (std::size_t i = 0; i < num_nodes_; ++i)
    {
        node_neighbors_[i].insert(i);
    }

    for (std::size_t e = 0; e < num_elements_; ++e)
    {
        auto &nodes_el = elements_[e];
        for (std::size_t i = 0; i < 3; i++)
        {
            std::size_t n1 = nodes_el[i];
            std::size_t n2 = nodes_el[(i + 1) % 3];
            node_neighbors_[n1].insert(n2);
            node_neighbors_[n2].insert(n1);
        }
    }
}

//------------------------------------------------------------------------------
void Mesh::printNodeNeighbors() const
{
    std::cout << "\nNode -> Neighboring Nodes:";
    for (std::size_t n = 0; n < num_nodes_; ++n)
    {
        std::cout << "\nNode " << n << ": ";
        for (auto nb : node_neighbors_[n])
            std::cout << nb << " ";
    }
    std::cout << "\n";
}

//------------------------------------------------------------------------------
bool Mesh::pointInTriangle(const Coord &p, const Coord &a, const Coord &b, const Coord &c) const
{
    // calculate cross products bp x ba, cp x cb, ap x ac
    double d1 = (p.x_ - b.x_) * (a.y_ - b.y_) - (p.y_ - b.y_) * (a.x_ - b.x_);
    double d2 = (p.x_ - c.x_) * (b.y_ - c.y_) - (p.y_ - c.y_) * (b.x_ - c.x_);
    double d3 = (p.x_ - a.x_) * (c.y_ - a.y_) - (p.y_ - a.y_) * (c.x_ - a.x_);

    // sign of the cross products
    bool sign_negative = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool sign_positive = (d1 > 0) || (d2 > 0) || (d3 > 0);

    // the sign of all cross products has to be equal
    return !(sign_negative && sign_positive);
}

//------------------------------------------------------------------------------
double Mesh::pointToSegmentDistance(const Coord &p, const Coord &a, const Coord &b) const
{
    Coord ab{b.x_ - a.x_, b.y_ - a.y_};
    Coord ap{p.x_ - a.x_, p.y_ - a.y_};

    double t = (ap.x_ * ab.x_ + ap.y_ * ab.y_) / (ab.x_ * ab.x_ + ab.y_ * ab.y_);
    t = std::max(0.0, std::min(1.0, t)); // clamping t to [0,1]

    Coord projection{a.x_ + t * ab.x_, a.y_ + t * ab.y_};
    return std::hypot(p.x_ - projection.x_, p.y_ - projection.y_); // calculating the norm |p - projection|
}

//------------------------------------------------------------------------------
double Mesh::pointToTriangleDistance(const Coord &p, const Coord &a, const Coord &b, const Coord &c) const
{
    if (pointInTriangle(p, a, b, c))
        return 0.0;

    double d1 = pointToSegmentDistance(p, a, b);
    double d2 = pointToSegmentDistance(p, b, c);
    double d3 = pointToSegmentDistance(p, c, a);

    return std::min({d1, d2, d3});
}

//------------------------------------------------------------------------------
std::pair<std::size_t, double> Mesh::findElementShortestDistanceToPoint(const Coord &p) const
{
    std::size_t e_min = 0;
    double dist_min = 0.0;

    for (std::size_t e = 0; e < num_elements_; ++e)
    {
        const Coord &a = nodes_[elements_[e][0]];
        const Coord &b = nodes_[elements_[e][1]];
        const Coord &c = nodes_[elements_[e][2]];

        double dist = pointToTriangleDistance(p, a, b, c);

        if (e == 0)
            dist_min = dist;

        if (dist < dist_min)
        {
            dist_min = dist;
            e_min = e;
        }
    }

    return std::make_pair(e_min, dist_min);
}
