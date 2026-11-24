/**
 * @file main.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Demonstrates usage of Mesh, Graph, WeightedGraph, and SparseMatrixCRS classes.
 */

#include <array>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "matrices-meshes-fem/coord.h"
#include "matrices-meshes-fem/mesh.h"
#include "matrices-meshes-fem/graph.h"
#include "matrices-meshes-fem/weightedgraph.h"
#include "matrices-meshes-fem/sparsematrixcrs.h"

/**
 * @brief Demonstrates usage of Mesh, Graph, WeightedGraph, and SparseMatrixCRS classes.
 *
 * The output is separated to different tasks. In particular, this program provides the following functionality:
 * - Reading mesh nodes and elements from files
 * - Printing mesh summary and mappings (node-to-elements, edge-to-elements, elements-to-neighboring-elements, nodes-to-neighboring-nodes)
 * - Finding the triangular element with the shortest distances for several points (inside or outside the domain)
 * - Derive directed and weighted graphs from the node-to-neighboring-nodes mapping
 * - Derive sparse matrix in CRS format from a weighted graph
 * - Performing matrix-vector multiplication
 * - Reading/writing sparse matrices from/to ASCII and binary files
 * - Visualize sparsity pattern of the matrix
 *
 * Usage:
 * @code
 * ./example_main coordinates_file elements_file
 * @endcode
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments. Expects:
 *   - argv[1]: Path to the coordinates file.
 *   - argv[2]: Path to the elements file.
 *
 * @return Exit status (0 on success, 1 on error)
 */
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " coordinates_file elements_file\n";
        return 1;
    }
    std::string const coords_file = argv[1];
    std::string const elements_file = argv[2];

    std::cout << "\n------------------------------------ Input files --------------------------------\n";
    std::cout << "Coordinates file: " << coords_file << '\n';
    std::cout << "Elements file: " << elements_file << '\n';

    std::cout << "\n------------------------------------ Mesh summary -------------------------------\n";
    try
    {
        Mesh mesh(coords_file, elements_file);
        mesh.printSummary();
        std::cout << "-------------------------------------------------------------------------------\n";
        std::cout << "Press Enter to continue...";
        std::cin.get();

        std::cout << "\n------------------------------------ Mesh mappings ------------------------------\n";
        mesh.printNodeToElements();

        mesh.printEdgeToElements();

        mesh.printElementNeighbors();

        mesh.printNodeNeighbors();
        std::cout << "-------------------------------------------------------------------------------\n";
        std::cout << "Press Enter to continue...";
        std::cin.get();

        std::cout << "\n------------------------------------ Point-to-element distances -----------------\n";
        constexpr std::array<Coord, 7> points{{{0.2, 0.3},
                                               {0.0, 0.333333},
                                               {0.2, -1.0},
                                               {2.0, 0.3},
                                               {0.2, 3.0},
                                               {-0.2, 0.3},
                                               {-1.0, -2.0}}};

        for (Coord const &p : points)
        {
            std::pair<std::size_t, double> const element_distance_min = mesh.findElementShortestDistanceToPoint(p);
            std::cout << "(" << p.x_ << "," << p.y_ << "): "
                 << "distance=" << element_distance_min.second
                 << ", closest element=" << element_distance_min.first
                 << "\n";
        }
        std::cout << "-------------------------------------------------------------------------------\n";
        std::cout << "Press Enter to continue...";
        std::cin.get();

        std::cout << "\n------------------------------------ Directed graph -----------------------------\n";
        Graph g(mesh.getNodeNeighbors());
        g.print();
        std::cout << "-------------------------------------------------------------------------------\n";
        std::cout << "Press Enter to continue...";
        std::cin.get();

        std::cout << "\n------------------------------------ Weighted graph -----------------------------\n";
        WeightedGraph wg(mesh.getNodeNeighbors(), mesh.getNodes());
        wg.print();
        std::cout << "-------------------------------------------------------------------------------\n";
        std::cout << "Press Enter to continue...";
        std::cin.get();

        std::cout << "\n------------------------------------ CRS matrix from weighted graph -------------\n";
        SparseMatrixCRS matrix(wg);
        matrix.print();
        matrix.printSparsityPattern();
        std::cout << "-------------------------------------------------------------------------------\n";
        std::cout << "Press Enter to continue...";
        std::cin.get();

        std::cout << "\n------------------------------------ Matrix-vector multiplication ---------------\n";
        std::vector<std::vector<std::pair<std::size_t, double>>> const entries_per_row = {
            {{1, -3.2}, {3, 2.1}},
            {{2, -5.1}},
            {{0, 7.2}, {1, 6.2}, {2, -4.4}, {3, 2.3}},
            {{0, -2.0}, {1, 3.0}, {3, 2.0}}};
        std::size_t const num_rows = 4;
        std::size_t const num_cols = 4;
        SparseMatrixCRS matrix_test(num_rows, num_cols, entries_per_row);
        matrix_test.print();

        std::vector<double> const x = {-1.2, 2.3, 3.4, -4.2};
        try
        {
            std::vector<double> const y = matrix_test.multiply(x);

            std::cout << "Input vector x: [";
            for (double const val : x)
                std::cout << val << " ";
            std::cout << "]";
            std::cout << "\nResult y = A * x: [";
            for (double const val : y)
                std::cout << val << " ";
            std::cout << "]";
            std::cout << "\nExpected result:  [-16.18 -17.34 -19 0.9 ]\n";
        }
        catch (std::runtime_error const &e)
        {
            std::cerr << "Error matrix-vector multiplication: " << e.what() << "\n";
        }

        std::cout << "\n------------------------------------ ASCII file I/O -----------------------------\n";
        try
        {
            std::cout << "Writing to ASCII file matrix_test.txt.\n";
            matrix_test.writeASCII("matrix_test.txt");
            std::cout << "Reading from ASCII file matrix_test.txt.\n";
            SparseMatrixCRS matrix_test_ascii(0, 0);
            matrix_test_ascii.readASCII("matrix_test.txt");
            matrix_test_ascii.print();
        }
        catch (std::runtime_error const &e)
        {
            std::cerr << "Error writing/reading ASCII file matrix_test.txt: " << e.what() << "\n";
        }

        std::cout << "\n------------------------------------ Binary file I/O ----------------------------\n";
        try
        {
            std::cout << "Writing to binary file matrix_test.bin.\n";
            matrix_test.writeBinary("matrix_test.bin");
            std::cout << "Reading from binary file matrix_test.bin.\n";
            SparseMatrixCRS matrix_test_binary(0, 0);
            matrix_test_binary.readBinary("matrix_test.bin");
            matrix_test_binary.print();
        }
        catch (std::runtime_error const &e)
        {
            std::cerr << "Error writing/reading binary file matrix_test.bin: " << e.what() << "\n";
        }
        std::cout << "-------------------------------------------------------------------------------\n";
    }
    catch (std::runtime_error const &e)
    {
        std::cerr << "Error creating mesh: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
