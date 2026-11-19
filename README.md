# Matrices Meshes FEM

---

A small C++20 library for working with 2D triangular meshes and graph-based sparse matrices. It reads mesh nodes and elements from files, derives node-neighbor, edge-to-element and element-neighbor mappings, builds directed and distance-weighted graphs, and assembles a sparse matrix in Compressed Row Storage (CRS) format with matrix-vector multiplication and ASCII/binary file I/O.

## Requirements

- CMake 3.28 or newer
- A C++20 compiler

## Project structure

```text
.
├── CMakeLists.txt
├── cmake/                       # CMake package config template
├── data/meshes/                 # sample 3x3 triangular mesh
├── examples/main.cpp            # demo executable
├── include/matrices-meshes-fem/ # public headers
├── src/                         # implementation
└── tests/                       # reserved for future tests
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

The default build type is `Release`. To override it or enable the optional optimizations:

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_NATIVE_ARCH=ON \
    -DENABLE_FAST_MATH=ON
cmake --build build
```

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
