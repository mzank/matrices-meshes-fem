/**
 * @file sparsematrixcrs.h
 * @author Marco Zank
 * @version 0.1
 * @date 19.11.2026
 * @brief Represents a sparse matrix using the Compressed Row Storage (CRS) format.
 */

#ifndef SPARSEMATRIXCRS_H_INCLUDED
#define SPARSEMATRIXCRS_H_INCLUDED

#include <cstddef>
#include <vector>
#include <string>
#include <utility>

#include "weightedgraph.h"

/**
 * @class SparseMatrixCRS
 * @brief Represents a sparse matrix using the Compressed Row Storage (CRS) format.
 *
 * This class provides the construction of the sparse matrix, matrix-vector multiplication,
 * printing and visualization of the sparsity pattern, and reading/writing in both ASCII
 * and binary formats.
 */
class SparseMatrixCRS
{
public:
    /**
     * @brief Constructs an empty sparse matrix with the specified number of rows and columns.
     * @param rows Number of rows.
     * @param cols Number of columns.
     */
    SparseMatrixCRS(std::size_t rows, std::size_t cols);

    /**
     * @brief Constructs a sparse matrix from a list of non-zero entries for each row.
     *
     * This constructor initializes a sparse matrix of size rows × cols using
     * a vector of non-zero entries provided row by row.
     *
     * @param rows Number of rows.
     * @param cols Number of columns.
     * @param entries_per_row A vector where each element is a vector of (column index, value)
     *        pairs representing the non-zero entries in that row.
     *
     * @note The column indices must be sorted for each row in entries_per_row.
     *
     * ### Example:
     * For a matrix:
     * ```
     * [ 2 0 1 ]
     * [ 0 0 0 ]
     * [ 0 3 0 ]
     * ```
     * The `entries_per_row` input might be:
     * ```
     * {
     *     { {0, 2.0}, {2, 1.0} },
     *     { },
     *     { {1, 3.0} }
     * }
     * ```
     */
    SparseMatrixCRS(std::size_t rows,
                    std::size_t cols,
                    const std::vector<std::vector<std::pair<std::size_t, double>>> &entries_per_row);

    /**
     * @brief Constructs a sparse matrix from a weighted graph.
     *
     * This constructor initializes the sparse matrix, where each non-zero entry corresponds to
     * a weighted directed edge in the graph.
     *
     * @param graph Weighted graph, which provides the edge-to-weight mapping.
     *
     * @note The matrix is assumed to be square: `num_rows_ == num_cols_`,
     * equal to the number of nodes in the graph.
     */
    SparseMatrixCRS(const WeightedGraph &graph);

    /**
     * @brief Prints the matrix.
     */
    void print() const;

    /**
     * @brief Multiplies the matrix with a vector.
     *
     * @param x A vector to multiply with. Its size must match the number of matrix columns.
     * @return Resulting vector after multiplication.
     *
     * @throws std::runtime_error if the size of vector x does not match the number of matrix columns.
     */
    std::vector<double> multiply(const std::vector<double> &x) const;

    /**
     * @brief Writes the matrix to an ASCII file.
     *
     * @param filename The file path to write to.
     *
     * @throws std::runtime_error if the file cannot be opened for writing.
     */
    void writeASCII(const std::string &filename) const;

    /**
     * @brief Reads the matrix from an ASCII file.
     *
     * This function reads matrix data from a text file formatted as follows:
     * - `num_rows_` (size_t): number of rows in the matrix
     * - `num_cols_` (size_t): number of columns in the matrix
     * - `nnz` (size_t): number of non-zero elements
     * - `values_` (double[nnz]): non-zero values, space-separated
     * - `col_idx_` (size_t[nnz]): column indices of the non-zero values
     * - `row_ptr_` (size_t[num_rows_ + 1]): row pointer array for CRS format
     *
     * @param filename The file path to read from.
     *
     * @throws std::runtime_error if the file cannot be opened for reading.
     */
    void readASCII(const std::string &filename);

    /**
     * @brief Writes the matrix to a binary file.
     *
     * @param filename The file path to write to.
     *
     * @throws std::runtime_error if the file cannot be opened for writing.
     */
    void writeBinary(const std::string &filename) const;

    /**
     * @brief Reads the matrix from a binary file.
     *
     * This function reads matrix data from a binary file expected to be stored as follows:
     * - `num_rows_` (size_t): number of rows in the matrix
     * - `num_cols_` (size_t): number of columns in the matrix
     * - `nnz` (size_t): number of non-zero elements
     * - `values_` (double[nnz]): non-zero values
     * - `col_idx_` (size_t[nnz]): column indices for each non-zero value
     * - `row_ptr_` (size_t[num_rows_ + 1]): row pointer array for CRS format
     *
     * @param filename The file path to read from.
     *
     * @throws std::runtime_error if the file cannot be opened for reading.
     */
    void readBinary(const std::string &filename);

    /**
     * @brief Prints the sparsity pattern of the matrix
     *
     * The output is of the form:
     * - `*` for positions where a non-zero value exists.
     * - `0` for zero entries.
     */
    void printSparsityPattern() const;

private:
    std::size_t num_rows_;                            ///< Number of rows in the matrix.
    std::size_t num_cols_;                            ///< Number of columns in the matrix.
    std::vector<double> values_;                      ///< Non-zero values in the matrix.
    std::vector<std::size_t> col_idx_;                ///< Column indices corresponding to values_.
    std::vector<std::size_t> row_ptr_;                ///< Row pointer to values_ and col_idx_.
};

#endif // SPARSEMATRIXCRS_H_INCLUDED
