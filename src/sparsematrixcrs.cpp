/**
 * @file sparsematrixcrs.cpp
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a sparse matrix using the Compressed Row Storage (CRS) format.
 */

#include "matrices-meshes-fem/sparsematrixcrs.h"

#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

//-----------------------------------------------------------------------------
SparseMatrixCRS::SparseMatrixCRS(size_t rows, size_t cols)
    : num_rows_(rows), num_cols_(cols), values_(), col_idx_(), row_ptr_(rows + 1, 0)
{
}

//-----------------------------------------------------------------------------
SparseMatrixCRS::SparseMatrixCRS(size_t rows,
                                 size_t cols,
                                 const std::vector<std::vector<std::pair<size_t, double>>> &entries_per_row)
    : num_rows_(rows), num_cols_(cols), values_(), col_idx_(), row_ptr_()
{
    row_ptr_.push_back(0); // Start of first row

    for (const auto &row : entries_per_row)
    {
        for (const auto &entry : row)
        {
            size_t col = entry.first;
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
    : num_rows_(0), num_cols_(0), values_(), col_idx_(), row_ptr_()
{
    const auto &adjacency = graph.getAdjacency();
    const auto &weights = graph.getWeights();

    num_rows_ = adjacency.size();
    num_cols_ = adjacency.size(); // Assuming square matrix

    row_ptr_.resize(num_rows_ + 1, 0);

    for (size_t row = 0; row < num_rows_; ++row)
    {
        const auto &neighbors = adjacency.at(row);

        for (const size_t col : neighbors)
        {
            pair<size_t, size_t> edge{row, col};
            values_.push_back(weights.at(edge));
            col_idx_.push_back(col);
        }
        row_ptr_[row + 1] = values_.size();
    }
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::print() const
{
    cout << "CRS matrix:\n";
    for (size_t row = 0; row < num_rows_; ++row)
    {
        size_t start = row_ptr_[row];
        size_t end = row_ptr_[row + 1];
        cout << "row " << row << ": ";
        for (size_t i = start; i < end; ++i)
        {
            cout << "(col " << col_idx_[i] << ": " << values_[i] << ") ";
        }
        cout << "\n";
    }
}

//-----------------------------------------------------------------------------
vector<double> SparseMatrixCRS::multiply(const vector<double> &x) const
{
    if (x.size() != num_cols_)
        throw runtime_error("Matrix size and vector size do not fit.");

    vector<double> result(num_rows_, 0.0);

    for (size_t row = 0; row < num_rows_; ++row)
    {
        size_t start = row_ptr_[row];
        size_t end = row_ptr_[row + 1];
        for (size_t i = start; i < end; ++i)
        {
            result[row] += values_[i] * x[col_idx_[i]];
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::writeASCII(const string &filename) const
{
    ofstream f_out(filename);
    if (!f_out)
        throw runtime_error("Cannot open file for writing.");

    size_t nnz = values_.size();
    f_out << num_rows_ << " " << num_cols_ << " " << nnz << "\n";

    for (double v : values_)
        f_out << v << " ";
    f_out << "\n";

    for (size_t c : col_idx_)
        f_out << c << " ";
    f_out << "\n";

    for (size_t r : row_ptr_)
        f_out << r << " ";
    f_out << "\n";
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::readASCII(const string &filename)
{
    ifstream f_in(filename);
    if (!f_in)
        throw runtime_error("Cannot open file for reading.");

    size_t nnz;
    f_in >> num_rows_ >> num_cols_ >> nnz;

    values_.resize(nnz);
    col_idx_.resize(nnz);
    row_ptr_.resize(num_rows_ + 1);

    for (size_t i = 0; i < nnz; ++i)
        f_in >> values_[i];
    for (size_t i = 0; i < nnz; ++i)
        f_in >> col_idx_[i];
    for (size_t i = 0; i < num_rows_ + 1; ++i)
        f_in >> row_ptr_[i];
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::writeBinary(const string &filename) const
{
    ofstream f_out(filename, ios::binary);
    if (!f_out)
        throw runtime_error("Cannot open file for writing.");

    size_t nnz = values_.size();
    f_out.write(reinterpret_cast<const char *>(&num_rows_), sizeof(size_t));
    f_out.write(reinterpret_cast<const char *>(&num_cols_), sizeof(size_t));
    f_out.write(reinterpret_cast<const char *>(&nnz), sizeof(size_t));

    f_out.write(reinterpret_cast<const char *>(values_.data()), nnz * sizeof(double));
    f_out.write(reinterpret_cast<const char *>(col_idx_.data()), nnz * sizeof(size_t));
    f_out.write(reinterpret_cast<const char *>(row_ptr_.data()), (num_rows_ + 1) * sizeof(size_t));
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::readBinary(const string &filename)
{
    ifstream f_in(filename, ios::binary);
    if (!f_in)
        throw runtime_error("Cannot open file for reading.");

    size_t nnz;
    f_in.read(reinterpret_cast<char *>(&num_rows_), sizeof(size_t));
    f_in.read(reinterpret_cast<char *>(&num_cols_), sizeof(size_t));
    f_in.read(reinterpret_cast<char *>(&nnz), sizeof(size_t));

    values_.resize(nnz);
    col_idx_.resize(nnz);
    row_ptr_.resize(num_rows_ + 1);

    f_in.read(reinterpret_cast<char *>(values_.data()), nnz * sizeof(double));
    f_in.read(reinterpret_cast<char *>(col_idx_.data()), nnz * sizeof(size_t));
    f_in.read(reinterpret_cast<char *>(row_ptr_.data()), (num_rows_ + 1) * sizeof(size_t));
}

//-----------------------------------------------------------------------------
void SparseMatrixCRS::printSparsityPattern() const
{
    cout << "\nSparsity pattern (rows x columns):\n";
    for (size_t row = 0; row < num_rows_; ++row)
    {
        size_t start = row_ptr_[row];
        size_t end = row_ptr_[row + 1];
        size_t nz_idx = start;
        for (size_t col = 0; col < num_cols_; ++col)
        {
            if (nz_idx < end && col_idx_[nz_idx] == col)
            {
                cout << "* ";
                ++nz_idx;
            }
            else
            {
                cout << "0 ";
            }
        }
        cout << "\n";
    }
}
