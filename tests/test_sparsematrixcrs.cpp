#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include "matrices-meshes-fem/coord.h"
#include "matrices-meshes-fem/sparsematrixcrs.h"
#include "matrices-meshes-fem/weightedgraph.h"

namespace
{
    std::string uniqueSuffix()
    {
        return std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    struct TempFile
    {
        explicit TempFile(const std::string &name)
            : path(std::filesystem::temp_directory_path() / ("mmf_sparse_" + name + "_" + uniqueSuffix() + ".dat"))
        {
        }

        ~TempFile()
        {
            std::error_code error;
            std::filesystem::remove(path, error);
        }

        std::filesystem::path path;
    };

    std::filesystem::path missingPath(const std::string &name)
    {
        return std::filesystem::temp_directory_path() / ("mmf_missing_" + name + "_" + uniqueSuffix() + ".dat");
    }

    // [ 2 0 1 ]
    // [ 0 0 0 ]
    // [ 0 3 0 ]
    SparseMatrixCRS sampleMatrix()
    {
        return SparseMatrixCRS(3, 3, {{{0, 2.0}, {2, 1.0}}, {}, {{1, 3.0}}});
    }

    WeightedGraph sampleWeightedGraph()
    {
        const std::vector<std::set<std::size_t>> neighbors{{0, 1, 2}, {0, 1}, {0}};
        const std::vector<Coord> coords{{0.0, 0.0}, {1.0, 0.0}, {0.0, 2.0}};
        return WeightedGraph(neighbors, coords);
    }

    void requireEqual(const std::vector<double> &actual, const std::vector<double> &expected)
    {
        REQUIRE(actual.size() == expected.size());
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            REQUIRE(actual[i] == Catch::Approx(expected[i]));
        }
    }
} // namespace

TEST_CASE("SparseMatrixCRS multiplies rows by the vector", "[sparsematrixcrs]")
{
    const SparseMatrixCRS matrix = sampleMatrix();
    const std::vector<double> x{1.0, 2.0, 3.0};

    requireEqual(matrix.multiply(x), {5.0, 0.0, 6.0});
}

TEST_CASE("SparseMatrixCRS treats an empty matrix as zero", "[sparsematrixcrs]")
{
    const SparseMatrixCRS matrix(3, 2);

    requireEqual(matrix.multiply({1.0, 2.0}), {0.0, 0.0, 0.0});
}

TEST_CASE("SparseMatrixCRS rejects a vector with the wrong size", "[sparsematrixcrs]")
{
    const SparseMatrixCRS matrix = sampleMatrix();
    const std::vector<double> too_short{1.0, 2.0};

    REQUIRE_THROWS_WITH(matrix.multiply(too_short), "Matrix size and vector size do not fit.");
}

TEST_CASE("SparseMatrixCRS builds a square matrix from a weighted graph", "[sparsematrixcrs]")
{
    const SparseMatrixCRS matrix(sampleWeightedGraph());
    const std::vector<double> x{1.0, 2.0, 4.0};

    // Only edges listed in the adjacency become nonzeros; node 2 has no self-edge there.
    requireEqual(matrix.multiply(x), {1.75, -1.0, 0.25});
}

TEST_CASE("SparseMatrixCRS round-trips through ASCII", "[sparsematrixcrs]")
{
    TempFile file("ascii");
    const SparseMatrixCRS original = sampleMatrix();
    const std::vector<double> x{1.0, 2.0, 3.0};

    original.writeASCII(file.path.string());

    std::ifstream input(file.path);
    std::string header;
    std::getline(input, header);
    REQUIRE(header == "3 3 3");

    SparseMatrixCRS restored(1, 1);
    restored.readASCII(file.path.string());
    requireEqual(restored.multiply(x), original.multiply(x));
}

TEST_CASE("SparseMatrixCRS round-trips through binary", "[sparsematrixcrs]")
{
    TempFile file("binary");
    const SparseMatrixCRS original = sampleMatrix();
    const std::vector<double> x{1.0, 2.0, 3.0};

    original.writeBinary(file.path.string());

    SparseMatrixCRS restored(1, 1);
    restored.readBinary(file.path.string());
    requireEqual(restored.multiply(x), original.multiply(x));
}

TEST_CASE("SparseMatrixCRS reports file errors", "[sparsematrixcrs]")
{
    SparseMatrixCRS matrix = sampleMatrix();

    SECTION("reading missing files")
    {
        REQUIRE_THROWS_WITH(matrix.readASCII(missingPath("ascii").string()),
                            "Cannot open file for reading.");
        REQUIRE_THROWS_WITH(matrix.readBinary(missingPath("binary").string()),
                            "Cannot open file for reading.");
    }

    SECTION("writing to a nonexistent directory")
    {
        const auto path = std::filesystem::temp_directory_path() /
                          ("mmf_no_such_directory_" + uniqueSuffix()) / "matrix.dat";
        REQUIRE_THROWS_WITH(matrix.writeASCII(path.string()), "Cannot open file for writing.");
        REQUIRE_THROWS_WITH(matrix.writeBinary(path.string()), "Cannot open file for writing.");
    }
}

TEST_CASE("SparseMatrixCRS prints its entries and sparsity pattern", "[sparsematrixcrs]")
{
    const SparseMatrixCRS matrix = sampleMatrix();

    std::ostringstream output;
    std::streambuf *old_buffer = std::cout.rdbuf(output.rdbuf());
    matrix.print();
    matrix.printSparsityPattern();
    std::cout.rdbuf(old_buffer);

    const std::string text = output.str();
    REQUIRE(text.find("CRS matrix:") != std::string::npos);
    REQUIRE(text.find("row 0: (col 0: 2) (col 2: 1)") != std::string::npos);
    REQUIRE(text.find("row 1:") != std::string::npos);
    REQUIRE(text.find("Sparsity pattern (rows x columns):") != std::string::npos);
    REQUIRE(text.find("* 0 *") != std::string::npos);
    REQUIRE(text.find("0 0 0") != std::string::npos);
    REQUIRE(text.find("0 * 0") != std::string::npos);
}
