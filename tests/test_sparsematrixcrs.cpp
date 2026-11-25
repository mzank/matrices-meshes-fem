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
        explicit TempFile(std::string const &name)
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

    void writeTextFile(std::filesystem::path const &path, std::string const &content)
    {
        std::ofstream out(path);
        out << content;
    }

    std::filesystem::path missingPath(std::string const &name)
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
        std::vector<std::set<std::size_t>> const neighbors{{0, 1, 2}, {0, 1}, {0}};
        std::vector<Coord> const coords{{0.0, 0.0}, {1.0, 0.0}, {0.0, 2.0}};
        return WeightedGraph(neighbors, coords);
    }

    void requireEqual(std::vector<double> const &actual, std::vector<double> const &expected)
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
    SparseMatrixCRS const matrix = sampleMatrix();
    std::vector<double> const x{1.0, 2.0, 3.0};

    requireEqual(matrix.multiply(x), {5.0, 0.0, 6.0});
}

TEST_CASE("SparseMatrixCRS treats an empty matrix as zero", "[sparsematrixcrs]")
{
    SparseMatrixCRS const matrix(3, 2);

    requireEqual(matrix.multiply({1.0, 2.0}), {0.0, 0.0, 0.0});
}

TEST_CASE("SparseMatrixCRS rejects a vector with the wrong size", "[sparsematrixcrs]")
{
    SparseMatrixCRS const matrix = sampleMatrix();
    std::vector<double> const too_short{1.0, 2.0};

    REQUIRE_THROWS_WITH(matrix.multiply(too_short), "Matrix size and vector size do not fit.");
}

TEST_CASE("SparseMatrixCRS builds a square matrix from a weighted graph", "[sparsematrixcrs]")
{
    SparseMatrixCRS const matrix(sampleWeightedGraph());
    std::vector<double> const x{1.0, 2.0, 4.0};

    // Only edges listed in the adjacency become nonzeros; node 2 has no self-edge there.
    requireEqual(matrix.multiply(x), {1.75, -1.0, 0.25});
}

TEST_CASE("SparseMatrixCRS round-trips through ASCII", "[sparsematrixcrs]")
{
    TempFile file("ascii");
    SparseMatrixCRS const original = sampleMatrix();
    std::vector<double> const x{1.0, 2.0, 3.0};

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
    SparseMatrixCRS const original = sampleMatrix();
    std::vector<double> const x{1.0, 2.0, 3.0};

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
        auto const path = std::filesystem::temp_directory_path() /
                          ("mmf_no_such_directory_" + uniqueSuffix()) / "matrix.dat";
        REQUIRE_THROWS_WITH(matrix.writeASCII(path.string()), "Cannot open file for writing.");
        REQUIRE_THROWS_WITH(matrix.writeBinary(path.string()), "Cannot open file for writing.");
    }
}

TEST_CASE("SparseMatrixCRS rejects malformed CRS files", "[sparsematrixcrs]")
{
    SparseMatrixCRS matrix = sampleMatrix();
    std::vector<double> const x{1.0, 2.0, 3.0};
    auto const requireUnchanged = [&matrix, &x]
    {
        requireEqual(matrix.multiply(x), {5.0, 0.0, 6.0});
    };

    SECTION("ASCII header is truncated")
    {
        TempFile file("ascii_truncated_header");
        writeTextFile(file.path, "3 3");

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), "Invalid CRS file.");
        requireUnchanged();
    }

    SECTION("ASCII values are truncated")
    {
        TempFile file("ascii_truncated_values");
        writeTextFile(file.path, "2 2 2\n1\n0 1\n0 1 2\n");

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), "Invalid CRS file.");
        requireUnchanged();
    }

    SECTION("ASCII column index is out of range")
    {
        TempFile file("ascii_bad_column");
        writeTextFile(file.path, "2 2 1\n5\n2\n0 1 1\n");

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), "Invalid CRS file.");
        requireUnchanged();
    }

    SECTION("ASCII row pointer is not monotonic")
    {
        TempFile file("ascii_bad_row_ptr");
        writeTextFile(file.path, "2 2 2\n1 2\n0 1\n0 3 2 3\n");

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), "Invalid CRS file.");
        requireUnchanged();
    }

    SECTION("ASCII columns are unsorted")
    {
        TempFile file("ascii_unsorted_columns");
        writeTextFile(file.path, "2 2 2\n1 2\n1 0\n0 2 2\n");

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), "Invalid CRS file.");
        requireUnchanged();
    }

    SECTION("binary data is truncated")
    {
        TempFile file("binary_truncated");
        sampleMatrix().writeBinary(file.path.string());
        std::filesystem::resize_file(file.path, 40);

        REQUIRE_THROWS_WITH(matrix.readBinary(file.path.string()), "Invalid CRS file.");
        requireUnchanged();
    }

    SECTION("binary column index is out of range")
    {
        TempFile file("binary_bad_column");
        sampleMatrix().writeBinary(file.path.string());

        std::size_t const bad_column = 99;
        std::ofstream out(file.path, std::ios::binary | std::ios::in | std::ios::out);
        out.seekp(static_cast<std::streamoff>(3 * sizeof(std::size_t) + 3 * sizeof(double)));
        out.write(reinterpret_cast<char const *>(&bad_column), sizeof(bad_column));
        out.close();

        REQUIRE_THROWS_WITH(matrix.readBinary(file.path.string()), "Invalid CRS file.");
        requireUnchanged();
    }
}

TEST_CASE("SparseMatrixCRS prints its entries and sparsity pattern", "[sparsematrixcrs]")
{
    SparseMatrixCRS const matrix = sampleMatrix();

    std::ostringstream output;
    std::streambuf *old_buffer = std::cout.rdbuf(output.rdbuf());
    matrix.print();
    matrix.printSparsityPattern();
    std::cout.rdbuf(old_buffer);

    std::string const text = output.str();
    REQUIRE(text.find("CRS matrix:") != std::string::npos);
    REQUIRE(text.find("row 0: (col 0: 2) (col 2: 1)") != std::string::npos);
    REQUIRE(text.find("row 1:") != std::string::npos);
    REQUIRE(text.find("Sparsity pattern (rows x columns):") != std::string::npos);
    REQUIRE(text.find("* 0 *") != std::string::npos);
    REQUIRE(text.find("0 0 0") != std::string::npos);
    REQUIRE(text.find("0 * 0") != std::string::npos);
}
