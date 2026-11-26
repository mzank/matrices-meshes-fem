# Matrices Meshes FEM

---

A small C++20 library for working with 2D triangular meshes and graph-based sparse matrices. It reads mesh nodes and elements from files, derives node-to-element, node-neighbor, edge-to-element and element-neighbor mappings, builds directed and distance-weighted graphs, and assembles a sparse matrix in Compressed Row Storage (CRS) format with matrix-vector multiplication and ASCII/binary file I/O.

Binary matrix files store raw `std::size_t` and `double` representations and are intended for same-platform Linux use with a matching ABI/endianness; use ASCII I/O for portable exchange.

## Requirements

- CMake 3.28 or newer
- A C++20 compiler

## Project structure

```text
.
├── .github/workflows/           # CI workflow
├── CMakeLists.txt
├── cmake/                       # CMake package config template
├── data/meshes/                 # sample 3x3 triangular mesh
├── examples/main.cpp            # demo executable
├── include/matrices-meshes-fem/ # public headers
├── src/                         # implementation
└── tests/                       # Catch2 unit tests
```

## Build

```bash
cmake -S . -B build
cmake --build build --parallel
```

The default build type is `Release`. To override it or enable the optional optimizations:

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_NATIVE_ARCH=ON \
    -DENABLE_FAST_MATH=ON
cmake --build build --parallel
```

## Tests

Tests are written with [Catch2](https://github.com/catchorg/Catch2) and registered with CTest. Catch2 is downloaded automatically at configure time via `FetchContent`, so the first configure requires network access. The current suite covers the `Coord` struct (`tests/test_coord.cpp`), the `Mesh` class (`tests/test_mesh.cpp`) using both the sample mesh in `data/meshes/` and temporary test files, the `Graph` class (`tests/test_graph.cpp`), the `WeightedGraph` class (`tests/test_weightedgraph.cpp`), and the `SparseMatrixCRS` class (`tests/test_sparsematrixcrs.cpp`).

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

`BUILD_TESTING` defaults to `ON` through `include(CTest)`, so the configure flag above is optional. Use `-DBUILD_TESTING=OFF` to skip building the tests.

## Install

```bash
cmake --install build --prefix /path/to/prefix
```

Without `--prefix`, CMake installs to the configured `CMAKE_INSTALL_PREFIX` (by default `/usr/local`, which usually requires root permissions). The install contains the library, the public headers and the CMake package files; the example executable is not installed.

## Run the example

Run the demo from the build directory so its generated `matrix_test.txt` and `matrix_test.bin` files stay out of the source tree:

```bash
cd build
./example_main ../data/meshes/3x3_coords_2d.txt ../data/meshes/3x3_elements_2d.txt
```

The demo is interactive and pauses with `Press Enter to continue...` between sections.

## Use the installed library

```cmake
find_package(matrices_meshes_fem 0.1 REQUIRED CONFIG)
target_link_libraries(app PRIVATE matrices_meshes_fem::matrices_meshes_fem)
```

Configure the consuming project with `-DCMAKE_PREFIX_PATH=/path/to/prefix`.

---

## License

This project is licensed under the Apache License 2.0. See [LICENSE](LICENSE) for details.

You can also view the license here: https://www.apache.org/licenses/LICENSE-2.0

---
