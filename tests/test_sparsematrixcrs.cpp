#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unistd.h>
#include <utility>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include "capture_stdout.h"
#include "matrices-meshes-fem/coord.h"
#include "matrices-meshes-fem/sparsematrixcrs.h"
#include "matrices-meshes-fem/weightedgraph.h"

namespace
{
    std::string uniqueSuffix()
    {
        static std::atomic<unsigned long long> counter{0};
        static std::random_device random_device;

        auto const timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        return std::to_string(timestamp) + "_" + std::to_string(::getpid()) + "_" +
               std::to_string(random_device()) + "_" + std::to_string(counter.fetch_add(1));
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

    void writeBinaryFile(std::filesystem::path const &path,
                         std::size_t rows,
                         std::size_t cols,
                         std::vector<double> const &values,
                         std::vector<std::size_t> const &col_idx,
                         std::vector<std::size_t> const &row_ptr)
    {
        std::ofstream out(path, std::ios::binary);
        auto const write_size = [&out](std::size_t const value)
        {
            out.write(reinterpret_cast<char const *>(&value), sizeof(value));
        };

        write_size(rows);
        write_size(cols);
        write_size(values.size());

        for (double const value : values)
            out.write(reinterpret_cast<char const *>(&value), sizeof(value));
        for (std::size_t const col : col_idx)
            write_size(col);
        for (std::size_t const row : row_ptr)
            write_size(row);
    }

    std::filesystem::path missingPath(std::string const &name)
    {
        return std::filesystem::temp_directory_path() / ("mmf_missing_" + name + "_" + uniqueSuffix() + ".dat");
    }

    std::string cannotOpenForReading(std::filesystem::path const &path)
    {
        return "Cannot open file for reading: " + path.string();
    }

    std::string cannotOpenForWriting(std::filesystem::path const &path)
    {
        return "Cannot open file for writing: " + path.string();
    }

    std::string invalidCRSFile(std::filesystem::path const &path)
    {
        return "Invalid CRS file: " + path.string();
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

TEST_CASE("SparseMatrixCRS rejects unsorted or duplicate entries", "[sparsematrixcrs]")
{
    SECTION("descending column indices")
    {
        REQUIRE_THROWS_AS(SparseMatrixCRS(1, 3, {{{2, 1.0}, {0, 2.0}}}),
                          std::invalid_argument);
    }

    SECTION("duplicate column indices")
    {
        REQUIRE_THROWS_AS(SparseMatrixCRS(1, 3, {{{1, 1.0}, {1, 2.0}}}),
                          std::invalid_argument);
    }

    SECTION("column index out of range")
    {
        REQUIRE_THROWS_AS(SparseMatrixCRS(1, 2, {{{2, 1.0}}}), std::out_of_range);
    }
}

TEST_CASE("SparseMatrixCRS validates constructor arguments", "[sparsematrixcrs]")
{
    SECTION("too many entry rows")
    {
        REQUIRE_THROWS_WITH(SparseMatrixCRS(1, 3, {{{0, 1.0}}, {{0, 2.0}}}),
                            "Too many rows in entries_per_row");
    }

    SECTION("fewer entry rows than matrix rows leave trailing rows empty")
    {
        SparseMatrixCRS const matrix(3, 3, {{{0, 1.0}}});

        requireEqual(matrix.multiply({1.0, 2.0, 3.0}), {1.0, 0.0, 0.0});
    }
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

    // Diagonal weights are included even for node 2, which has no self-edge.
    requireEqual(matrix.multiply(x), {1.75, -1.0, -0.75});
}

TEST_CASE("SparseMatrixCRS from WeightedGraph includes all diagonal weights", "[sparsematrixcrs]")
{
    SparseMatrixCRS const matrix(sampleWeightedGraph());

    std::ostringstream output;
    [[maybe_unused]] mmf_test::StdoutCapture const capture(output);
    matrix.print();

    std::string const text = output.str();
    REQUIRE(text.find("row 0: (col 0: -1.25) (col 1: 1) (col 2: 0.25)") != std::string::npos);
    REQUIRE(text.find("row 1: (col 0: 1) (col 1: -1)") != std::string::npos);
    REQUIRE(text.find("row 2: (col 0: 0.25) (col 2: -0.25)") != std::string::npos);
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

TEST_CASE("SparseMatrixCRS round-trips empty and zero-dimension matrices", "[sparsematrixcrs]")
{
    auto const checkRoundTrip = [](std::size_t const rows, std::size_t const cols)
    {
        std::string const dimensions = std::to_string(rows) + "x" + std::to_string(cols);
        TempFile ascii("empty_ascii_" + dimensions);
        TempFile binary("empty_binary_" + dimensions);

        SparseMatrixCRS const original(rows, cols);
        std::vector<double> const x(cols, 2.0);
        auto const expected = original.multiply(x);

        original.writeASCII(ascii.path.string());
        original.writeBinary(binary.path.string());

        std::ifstream input(ascii.path);
        std::string header;
        std::getline(input, header);
        REQUIRE(header == std::to_string(rows) + " " + std::to_string(cols) + " 0");

        SparseMatrixCRS restored_ascii(1, 1);
        restored_ascii.readASCII(ascii.path.string());
        requireEqual(restored_ascii.multiply(x), expected);

        SparseMatrixCRS restored_binary(1, 1);
        restored_binary.readBinary(binary.path.string());
        requireEqual(restored_binary.multiply(x), expected);
    };

    SECTION("0x0")
    {
        checkRoundTrip(0, 0);
    }

    SECTION("0x3")
    {
        checkRoundTrip(0, 3);
    }

    SECTION("3x0")
    {
        checkRoundTrip(3, 0);
    }
}

TEST_CASE("SparseMatrixCRS reports file errors", "[sparsematrixcrs]")
{
    SparseMatrixCRS matrix = sampleMatrix();

    SECTION("reading missing files")
    {
        auto const ascii_path = missingPath("ascii");
        auto const binary_path = missingPath("binary");
        REQUIRE_THROWS_WITH(matrix.readASCII(ascii_path.string()),
                            cannotOpenForReading(ascii_path));
        REQUIRE_THROWS_WITH(matrix.readBinary(binary_path.string()),
                            cannotOpenForReading(binary_path));
    }

    SECTION("writing to a nonexistent directory")
    {
        auto const path = std::filesystem::temp_directory_path() /
                          ("mmf_no_such_directory_" + uniqueSuffix()) / "matrix.dat";
        REQUIRE_THROWS_WITH(matrix.writeASCII(path.string()), cannotOpenForWriting(path));
        REQUIRE_THROWS_WITH(matrix.writeBinary(path.string()), cannotOpenForWriting(path));
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

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), invalidCRSFile(file.path));
        requireUnchanged();
    }

    SECTION("ASCII values are truncated")
    {
        TempFile file("ascii_truncated_values");
        writeTextFile(file.path, "2 2 2\n1\n0 1\n0 1 2\n");

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), invalidCRSFile(file.path));
        requireUnchanged();
    }

    SECTION("ASCII column index is out of range")
    {
        TempFile file("ascii_bad_column");
        writeTextFile(file.path, "2 2 1\n5\n2\n0 1 1\n");

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), invalidCRSFile(file.path));
        requireUnchanged();
    }

    SECTION("ASCII row pointer is not monotonic")
    {
        TempFile file("ascii_bad_row_ptr");
        writeTextFile(file.path, "2 2 2\n1 2\n0 1\n0 3 2 3\n");

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), invalidCRSFile(file.path));
        requireUnchanged();
    }

    SECTION("ASCII columns are unsorted")
    {
        TempFile file("ascii_unsorted_columns");
        writeTextFile(file.path, "2 2 2\n1 2\n1 0\n0 2 2\n");

        REQUIRE_THROWS_WITH(matrix.readASCII(file.path.string()), invalidCRSFile(file.path));
        requireUnchanged();
    }

    SECTION("binary data is truncated")
    {
        TempFile file("binary_truncated");
        sampleMatrix().writeBinary(file.path.string());
        std::filesystem::resize_file(file.path, 40);

        REQUIRE_THROWS_WITH(matrix.readBinary(file.path.string()), invalidCRSFile(file.path));
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

        REQUIRE_THROWS_WITH(matrix.readBinary(file.path.string()), invalidCRSFile(file.path));
        requireUnchanged();
    }

    SECTION("binary row pointer is not monotonic")
    {
        TempFile file("binary_bad_row_ptr");
        writeBinaryFile(file.path, 2, 2, {1.0, 2.0}, {0, 1}, {0, 2, 1});

        REQUIRE_THROWS_WITH(matrix.readBinary(file.path.string()), invalidCRSFile(file.path));
        requireUnchanged();
    }

    SECTION("binary columns are unsorted")
    {
        TempFile file("binary_unsorted_columns");
        writeBinaryFile(file.path, 2, 2, {1.0, 2.0}, {1, 0}, {0, 2, 2});

        REQUIRE_THROWS_WITH(matrix.readBinary(file.path.string()), invalidCRSFile(file.path));
        requireUnchanged();
    }
}

TEST_CASE("SparseMatrixCRS prints its entries and sparsity pattern", "[sparsematrixcrs]")
{
    SparseMatrixCRS const matrix = sampleMatrix();

    std::ostringstream output;
    [[maybe_unused]] mmf_test::StdoutCapture const capture(output);
    matrix.print();
    matrix.printSparsityPattern();

    std::string const text = output.str();
    REQUIRE(text.find("CRS matrix:") != std::string::npos);
    REQUIRE(text.find("row 0: (col 0: 2) (col 2: 1)") != std::string::npos);
    REQUIRE(text.find("row 1:") != std::string::npos);
    REQUIRE(text.find("Sparsity pattern (rows x columns):") != std::string::npos);
    REQUIRE(text.find("* 0 *") != std::string::npos);
    REQUIRE(text.find("0 0 0") != std::string::npos);
    REQUIRE(text.find("0 * 0") != std::string::npos);
}
