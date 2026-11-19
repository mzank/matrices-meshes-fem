/**
 * @file main.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Demonstrates usage of Mesh, Graph, WeightedGraph, and SparseMatrixCRS classes.
 */

#include <iostream>
#include <string>

#include "matrices-meshes-fem/coord.h"
#include "matrices-meshes-fem/mesh.h"
#include "matrices-meshes-fem/graph.h"
#include "matrices-meshes-fem/weightedgraph.h"
#include "matrices-meshes-fem/sparsematrixcrs.h"

using namespace std;

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
 * ./main coordinates_file elements_file
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
        cerr << "Usage: " << argv[0] << " coordinates_file elements_file\n";
        return 1;
    }
    string coords_file = argv[1];
    string elements_file = argv[2];

    cout << "\n------------------------------------ Input files --------------------------------\n";
    cout << "Coordinates file: " << coords_file << '\n';
    cout << "Elements file: " << elements_file << '\n';

    cout << "\n------------------------------------ Mesh summary -------------------------------\n";
    try
    {
        Mesh mesh(coords_file, elements_file);
        mesh.printSummary();
        cout << "-------------------------------------------------------------------------------\n";
        cout << "Press Enter to continue...";
        cin.get();

        cout << "\n------------------------------------ Mesh mappings ------------------------------\n";
        mesh.printNodeToElements();

        mesh.printEdgeToElements();

        mesh.printElementNeighbors();

        mesh.printNodeNeighbors();
        cout << "-------------------------------------------------------------------------------\n";
        cout << "Press Enter to continue...";
        cin.get();

        cout << "\n------------------------------------ Point-to-element distances -----------------\n";
        Coord points[7] = {{0.2, 0.3},
                           {0.0, 0.333333},
                           {0.2, -1.0},
                           {2.0, 0.3},
                           {0.2, 3.0},
                           {-0.2, 0.3},
                           {-1.0, -2.0}};

        for (size_t i = 0; i < 7; ++i)
        {
            Coord p = points[i];
            pair<size_t, double> element_distance_min = mesh.findElementShortestDistanceToPoint(p);
            cout << "(" << p.x_ << "," << p.y_ << "): "
                 << "distance=" << element_distance_min.second
                 << ", closest element=" << element_distance_min.first
                 << "\n";
        }
        cout << "-------------------------------------------------------------------------------\n";
        cout << "Press Enter to continue...";
        cin.get();

        cout << "\n------------------------------------ Directed graph -----------------------------\n";
        Graph g(mesh.getNodeNeighbors());
        g.print();
        cout << "-------------------------------------------------------------------------------\n";
        cout << "Press Enter to continue...";
        cin.get();

        cout << "\n------------------------------------ Weighted graph -----------------------------\n";
        WeightedGraph wg(mesh.getNodeNeighbors(), mesh.getNodes());
        wg.print();
        cout << "-------------------------------------------------------------------------------\n";
        cout << "Press Enter to continue...";
        cin.get();

        cout << "\n------------------------------------ CRS matrix from weighted graph -------------\n";
        SparseMatrixCRS matrix(wg);
        matrix.print();
        matrix.printSparsityPattern();
        cout << "-------------------------------------------------------------------------------\n";
        cout << "Press Enter to continue...";
        cin.get();

        cout << "\n------------------------------------ Matrix-vector multiplication ---------------\n";
        vector<vector<pair<size_t, double>>> entries_per_row = {
            {{1, -3.2}, {3, 2.1}},
            {{2, -5.1}},
            {{0, 7.2}, {1, 6.2}, {2, -4.4}, {3, 2.3}},
            {{0, -2.0}, {1, 3.0}, {3, 2.0}}};
        size_t num_rows = 4;
        size_t num_cols = 4;
        SparseMatrixCRS matrix_test(num_rows, num_cols, entries_per_row);
        matrix_test.print();

        vector<double> x = {-1.2, 2.3, 3.4, -4.2};
        try
        {
            vector<double> y = matrix_test.multiply(x);

            cout << "Input vector x: [";
            for (double val : x)
                cout << val << " ";
            cout << "]";
            cout << "\nResult y = A * x: [";
            for (double val : y)
                cout << val << " ";
            cout << "]";
            cout << "\nExpected result:  [-16.18 -17.34 -19 0.9 ]\n";
        }
        catch (const runtime_error &e)
        {
            cerr << "Error matrix-vector multiplication: " << e.what() << endl;
        }

        cout << "\n------------------------------------ ASCII file I/O -----------------------------\n";
        try
        {
            cout << "Writing to ASCII file matrix_test.txt.\n";
            matrix_test.writeASCII("matrix_test.txt");
            cout << "Reading from ASCII file matrix_test.txt.\n";
            SparseMatrixCRS matrix_test_ascii(0, 0);
            matrix_test_ascii.readASCII("matrix_test.txt");
            matrix_test_ascii.print();
        }
        catch (const runtime_error &e)
        {
            cerr << "Error writing/reading ASCII file matrix_test.txt: " << e.what() << endl;
        }

        cout << "\n------------------------------------ Binary file I/O ----------------------------\n";
        try
        {
            cout << "Writing to binary file matrix_test.bin.\n";
            matrix_test.writeBinary("matrix_test.bin");
            cout << "Reading from binary file matrix_test.bin.\n";
            SparseMatrixCRS matrix_test_binary(0, 0);
            matrix_test_binary.readBinary("matrix_test.bin");
            matrix_test_binary.print();
        }
        catch (const runtime_error &e)
        {
            cerr << "Error writing/reading binary file matrix_test.bin: " << e.what() << endl;
        }
        cout << "-------------------------------------------------------------------------------\n";
    }
    catch (const runtime_error &e)
    {
        cerr << "Error creating mesh: " << e.what() << endl;
        return 1;
    }

    return 0;
}
