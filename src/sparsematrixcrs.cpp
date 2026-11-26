/**
 * @file sparsematrixcrs.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a sparse matrix using the Compressed Row Storage (CRS) format.
 */

#include "matrices-meshes-fem/sparsematrixcrs.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>

//-----------------------------------------------------------------------------
SparseMatrixCRS::SparseMatrixCRS(std::size_t rows, std::size_t cols)
    : num_rows_(rows), num_cols_(cols), values_(), col_idx_(), row_ptr_(rows + 1, 0)
{
}

//-----------------------------------------------------------------------------
SparseMatrixCRS::SparseMatrixCRS(
    std::size_t rows,
    std::size_t cols,
    std::vector<std::vector<std::pair<std::size_t, double>>> const &entries_per_row)
    : num_rows_(rows),
      num_cols_(cols),
      values_(),
      col_idx_(),
      row_ptr_{0} // initial row offset
{
    // Validate the number of supplied rows.
    if (entries_per_row.size() > rows)
    {
        throw std::invalid_argument("Too many rows in entries_per_row");
    }

    // Calculate the total number of non-zero elements.
    auto const nnz = std::accumulate(
        entries_per_row.begin(),
        entries_per_row.end(),
        std::size_t{0},
        [](std::size_t sum, auto const &row)
        {
            return sum + row.size();
        });

    // Reserve memory to avoid unnecessary reallocations.
    values_.reserve(nnz);
    col_idx_.reserve(nnz);
    row_ptr_.reserve(rows + 1);

    // Process each row.
    for (auto const &row : entries_per_row)
    {
        std::size_t previous_col{};
        bool first_entry = true;

        for (auto const &[col, value] : row)
        {
            // Validate column index.
            if (col >= cols)
            {
                throw std::out_of_range("Column index out of range");
            }

            // printSparsityPattern() and the CRS format require sorted, unique columns.
            if (!first_entry && col <= previous_col)
            {
                throw std::invalid_argument("Column indices must be strictly increasing");
            }

            values_.push_back(value);
            col_idx_.push_back(col);
            previous_col = col;
            first_entry = false;
        }

        // Store the end position of the current row.
        row_ptr_.push_back(values_.size());
    }

    // Handle missing (empty) rows.
    row_ptr_.resize(rows + 1, values_.size());
}

//-----------------------------------------------------------------------------
SparseMatrixCRS::SparseMatrixCRS(WeightedGraph const &graph)
    : num_rows_(graph.getAdjacency().size()),
      num_cols_(num_rows_), // Assuming square matrix
      values_(),
      col_idx_(),
      row_ptr_{0} // initial row offset
{
    auto const &weights = graph.getWeights();

    // The weight map contains every adjacency weight plus one diagonal per node.
    auto const nnz = weights.size();

    // Reserve memory to avoid unnecessary reallocations.
    values_.reserve(nnz);
    col_idx_.reserve(nnz);
    row_ptr_.reserve(num_rows_ + 1);

    // Process each row. std::map orders entries by (row, column), so columns stay sorted.
    for (std::size_t row = 0; row < num_rows_; ++row)
    {
        for (auto it = weights.lower_bound({row, 0});
             it != weights.end() && it->first.first == row; ++it)
        {
            auto const col = it->first.second;
            if (col >= num_cols_)
                throw std::out_of_range("Column index out of range");

            values_.push_back(it->second);
            col_idx_.push_back(col);
        }

        // Store the end position of the current row.
        row_ptr_.push_back(values_.size());
    }

    // Handle missing (empty) rows.
    row_ptr_.resize(num_rows_ + 1, values_.size());
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::print() const
{
    std::cout << "CRS matrix:\n";
    for (std::size_t row = 0; row < num_rows_; ++row)
    {
        std::size_t const start = row_ptr_[row];
        std::size_t const end = row_ptr_[row + 1];
        std::cout << "row " << row << ": ";
        for (std::size_t i = start; i < end; ++i)
        {
            std::cout << "(col " << col_idx_[i] << ": " << values_[i] << ") ";
        }
        std::cout << "\n";
    }
}

//-----------------------------------------------------------------------------
std::vector<double> SparseMatrixCRS::multiply(std::vector<double> const &x) const
{
    if (x.size() != num_cols_)
        throw std::runtime_error("Matrix size and vector size do not fit.");

    std::vector<double> result(num_rows_, 0.0);

    for (std::size_t row = 0; row < num_rows_; ++row)
    {
        std::size_t const start = row_ptr_[row];
        std::size_t const end = row_ptr_[row + 1];
        for (std::size_t i = start; i < end; ++i)
        {
            result[row] += values_[i] * x[col_idx_[i]];
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::writeASCII(std::string const &filename) const
{
    std::ofstream f_out(filename);
    if (!f_out)
        throw std::runtime_error("Cannot open file for writing: " + filename);

    std::size_t const nnz = values_.size();
    f_out << num_rows_ << " " << num_cols_ << " " << nnz << "\n";

    for (double const v : values_)
        f_out << v << " ";
    f_out << "\n";

    for (std::size_t const c : col_idx_)
        f_out << c << " ";
    f_out << "\n";

    for (std::size_t const r : row_ptr_)
        f_out << r << " ";
    f_out << "\n";
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::validateCRSData(std::size_t rows,
                                      std::size_t cols,
                                      std::vector<double> const &values,
                                      std::vector<std::size_t> const &col_idx,
                                      std::vector<std::size_t> const &row_ptr,
                                      std::string const &filename)
{
    auto const invalid = [&filename] { throw std::runtime_error("Invalid CRS file: " + filename); };

    if (rows == std::numeric_limits<std::size_t>::max() ||
        values.size() != col_idx.size() ||
        row_ptr.size() != rows + 1 ||
        row_ptr.front() != 0 ||
        row_ptr.back() != values.size())
    {
        invalid();
    }

    for (std::size_t const col : col_idx)
    {
        if (col >= cols)
            invalid();
    }

    for (std::size_t row = 0; row < rows; ++row)
    {
        std::size_t const begin = row_ptr[row];
        std::size_t const end = row_ptr[row + 1];
        if (begin > end || end > values.size())
            invalid();

        for (std::size_t i = begin + 1; i < end; ++i)
        {
            if (col_idx[i - 1] >= col_idx[i])
                invalid();
        }
    }
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::readASCII(std::string const &filename)
{
    std::ifstream f_in(filename);
    if (!f_in)
        throw std::runtime_error("Cannot open file for reading: " + filename);

    auto const invalid = [&filename] { throw std::runtime_error("Invalid CRS file: " + filename); };

    std::size_t new_rows{};
    std::size_t new_cols{};
    std::size_t nnz{};
    if (!(f_in >> new_rows >> new_cols >> nnz) || new_rows == std::numeric_limits<std::size_t>::max())
        invalid();

    std::vector<double> new_values(nnz);
    for (std::size_t i = 0; i < nnz; ++i)
    {
        if (!(f_in >> new_values[i]))
            invalid();
    }

    std::vector<std::size_t> new_col_idx(nnz);
    for (std::size_t i = 0; i < nnz; ++i)
    {
        if (!(f_in >> new_col_idx[i]))
            invalid();
    }

    std::vector<std::size_t> new_row_ptr(new_rows + 1);
    for (std::size_t i = 0; i < new_rows + 1; ++i)
    {
        if (!(f_in >> new_row_ptr[i]))
            invalid();
    }

    validateCRSData(new_rows, new_cols, new_values, new_col_idx, new_row_ptr, filename);

    num_rows_ = new_rows;
    num_cols_ = new_cols;
    values_ = std::move(new_values);
    col_idx_ = std::move(new_col_idx);
    row_ptr_ = std::move(new_row_ptr);
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::writeBinary(std::string const &filename) const
{
    std::ofstream f_out(filename, std::ios::binary);
    if (!f_out)
        throw std::runtime_error("Cannot open file for writing: " + filename);

    std::size_t const nnz = values_.size();
    f_out.write(reinterpret_cast<char const *>(&num_rows_), sizeof(std::size_t));
    f_out.write(reinterpret_cast<char const *>(&num_cols_), sizeof(std::size_t));
    f_out.write(reinterpret_cast<char const *>(&nnz), sizeof(std::size_t));

    f_out.write(reinterpret_cast<char const *>(values_.data()), nnz * sizeof(double));
    f_out.write(reinterpret_cast<char const *>(col_idx_.data()), nnz * sizeof(std::size_t));
    f_out.write(reinterpret_cast<char const *>(row_ptr_.data()), (num_rows_ + 1) * sizeof(std::size_t));
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::readBinary(std::string const &filename)
{
    std::ifstream f_in(filename, std::ios::binary);
    if (!f_in)
        throw std::runtime_error("Cannot open file for reading: " + filename);

    auto const invalid = [&filename] { throw std::runtime_error("Invalid CRS file: " + filename); };
    auto const read_exact = [&f_in, &invalid](void *data, std::streamsize count)
    {
        if (count > 0 && !f_in.read(reinterpret_cast<char *>(data), count))
            invalid();
    };

    std::size_t new_rows{};
    std::size_t new_cols{};
    std::size_t nnz{};
    read_exact(&new_rows, sizeof(new_rows));
    read_exact(&new_cols, sizeof(new_cols));
    read_exact(&nnz, sizeof(nnz));

    auto const max_stream_size = static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max());
    if (new_rows == std::numeric_limits<std::size_t>::max() ||
        nnz > max_stream_size / sizeof(double) ||
        nnz > max_stream_size / sizeof(std::size_t) ||
        new_rows + 1 > max_stream_size / sizeof(std::size_t))
    {
        invalid();
    }

    std::vector<double> new_values(nnz);
    read_exact(new_values.data(), static_cast<std::streamsize>(nnz * sizeof(double)));

    std::vector<std::size_t> new_col_idx(nnz);
    read_exact(new_col_idx.data(), static_cast<std::streamsize>(nnz * sizeof(std::size_t)));

    std::vector<std::size_t> new_row_ptr(new_rows + 1);
    read_exact(new_row_ptr.data(), static_cast<std::streamsize>((new_rows + 1) * sizeof(std::size_t)));

    validateCRSData(new_rows, new_cols, new_values, new_col_idx, new_row_ptr, filename);

    num_rows_ = new_rows;
    num_cols_ = new_cols;
    values_ = std::move(new_values);
    col_idx_ = std::move(new_col_idx);
    row_ptr_ = std::move(new_row_ptr);
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::printSparsityPattern() const
{
    std::cout << "\nSparsity pattern (rows x columns):\n";
    for (std::size_t row = 0; row < num_rows_; ++row)
    {
        std::size_t const start = row_ptr_[row];
        std::size_t const end = row_ptr_[row + 1];
        std::size_t nz_idx = start;
        for (std::size_t col = 0; col < num_cols_; ++col)
        {
            if (nz_idx < end && col_idx_[nz_idx] == col)
            {
                std::cout << "* ";
                ++nz_idx;
            }
            else
            {
                std::cout << "0 ";
            }
        }
        std::cout << "\n";
    }
}
