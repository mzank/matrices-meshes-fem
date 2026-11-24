/**
 * @file sparsematrixcrs.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a sparse matrix using the Compressed Row Storage (CRS) format.
 */

#include "matrices-meshes-fem/sparsematrixcrs.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>

//-----------------------------------------------------------------------------
SparseMatrixCRS::SparseMatrixCRS(std::size_t rows, std::size_t cols)
    : num_rows_(rows), num_cols_(cols), values_(), col_idx_(), row_ptr_(rows + 1, 0)
{
}

//-----------------------------------------------------------------------------
SparseMatrixCRS::SparseMatrixCRS(std::size_t rows,
                                 std::size_t cols,
                                 const std::vector<std::vector<std::pair<std::size_t, double>>> &entries_per_row)
    : num_rows_(rows), num_cols_(cols), values_(), col_idx_(), row_ptr_()
{
    row_ptr_.push_back(0); // Start of first row

    for (const auto &row : entries_per_row)
    {
        for (const auto &entry : row)
        {
            std::size_t col = entry.first;
            double val = entry.second;

            values_.push_back(val);
            col_idx_.push_back(col);
        }
        row_ptr_.push_back(values_.size()); // Mark end of this row (start of next)
    }

    while (row_ptr_.size() < num_rows_ + 1)
    {
        row_ptr_.push_back(values_.size());
    }
}

//-----------------------------------------------------------------------------
SparseMatrixCRS::SparseMatrixCRS(const WeightedGraph &graph)
    : num_rows_(graph.getAdjacency().size()),
      num_cols_(num_rows_), // Assuming square matrix
      values_(),
      col_idx_(),
      row_ptr_(num_rows_ + 1, 0)
{
    const auto &adjacency = graph.getAdjacency();
    const auto &weights = graph.getWeights();

    for (std::size_t row = 0; row < num_rows_; ++row)
    {
        const auto &neighbors = adjacency.at(row);

        for (const std::size_t col : neighbors)
        {
            std::pair<std::size_t, std::size_t> edge{row, col};
            values_.push_back(weights.at(edge));
            col_idx_.push_back(col);
        }
        row_ptr_[row + 1] = values_.size();
    }
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::print() const
{
    std::cout << "CRS matrix:\n";
    for (std::size_t row = 0; row < num_rows_; ++row)
    {
        std::size_t start = row_ptr_[row];
        std::size_t end = row_ptr_[row + 1];
        std::cout << "row " << row << ": ";
        for (std::size_t i = start; i < end; ++i)
        {
            std::cout << "(col " << col_idx_[i] << ": " << values_[i] << ") ";
        }
        std::cout << "\n";
    }
}

//-----------------------------------------------------------------------------
std::vector<double> SparseMatrixCRS::multiply(const std::vector<double> &x) const
{
    if (x.size() != num_cols_)
        throw std::runtime_error("Matrix size and vector size do not fit.");

    std::vector<double> result(num_rows_, 0.0);

    for (std::size_t row = 0; row < num_rows_; ++row)
    {
        std::size_t start = row_ptr_[row];
        std::size_t end = row_ptr_[row + 1];
        for (std::size_t i = start; i < end; ++i)
        {
            result[row] += values_[i] * x[col_idx_[i]];
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::writeASCII(const std::string &filename) const
{
    std::ofstream f_out(filename);
    if (!f_out)
        throw std::runtime_error("Cannot open file for writing.");

    std::size_t nnz = values_.size();
    f_out << num_rows_ << " " << num_cols_ << " " << nnz << "\n";

    for (double v : values_)
        f_out << v << " ";
    f_out << "\n";

    for (std::size_t c : col_idx_)
        f_out << c << " ";
    f_out << "\n";

    for (std::size_t r : row_ptr_)
        f_out << r << " ";
    f_out << "\n";
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::readASCII(const std::string &filename)
{
    std::ifstream f_in(filename);
    if (!f_in)
        throw std::runtime_error("Cannot open file for reading.");

    std::size_t nnz;
    f_in >> num_rows_ >> num_cols_ >> nnz;

    values_.resize(nnz);
    col_idx_.resize(nnz);
    row_ptr_.resize(num_rows_ + 1);

    for (std::size_t i = 0; i < nnz; ++i)
        f_in >> values_[i];
    for (std::size_t i = 0; i < nnz; ++i)
        f_in >> col_idx_[i];
    for (std::size_t i = 0; i < num_rows_ + 1; ++i)
        f_in >> row_ptr_[i];
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::writeBinary(const std::string &filename) const
{
    std::ofstream f_out(filename, std::ios::binary);
    if (!f_out)
        throw std::runtime_error("Cannot open file for writing.");

    std::size_t nnz = values_.size();
    f_out.write(reinterpret_cast<const char *>(&num_rows_), sizeof(std::size_t));
    f_out.write(reinterpret_cast<const char *>(&num_cols_), sizeof(std::size_t));
    f_out.write(reinterpret_cast<const char *>(&nnz), sizeof(std::size_t));

    f_out.write(reinterpret_cast<const char *>(values_.data()), nnz * sizeof(double));
    f_out.write(reinterpret_cast<const char *>(col_idx_.data()), nnz * sizeof(std::size_t));
    f_out.write(reinterpret_cast<const char *>(row_ptr_.data()), (num_rows_ + 1) * sizeof(std::size_t));
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::readBinary(const std::string &filename)
{
    std::ifstream f_in(filename, std::ios::binary);
    if (!f_in)
        throw std::runtime_error("Cannot open file for reading.");

    std::size_t nnz;
    f_in.read(reinterpret_cast<char *>(&num_rows_), sizeof(std::size_t));
    f_in.read(reinterpret_cast<char *>(&num_cols_), sizeof(std::size_t));
    f_in.read(reinterpret_cast<char *>(&nnz), sizeof(std::size_t));

    values_.resize(nnz);
    col_idx_.resize(nnz);
    row_ptr_.resize(num_rows_ + 1);

    f_in.read(reinterpret_cast<char *>(values_.data()), nnz * sizeof(double));
    f_in.read(reinterpret_cast<char *>(col_idx_.data()), nnz * sizeof(std::size_t));
    f_in.read(reinterpret_cast<char *>(row_ptr_.data()), (num_rows_ + 1) * sizeof(std::size_t));
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::printSparsityPattern() const
{
    std::cout << "\nSparsity pattern (rows x columns):\n";
    for (std::size_t row = 0; row < num_rows_; ++row)
    {
        std::size_t start = row_ptr_[row];
        std::size_t end = row_ptr_[row + 1];
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
