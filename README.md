# Matrices Meshes FEM

---

A small C++20 library for working with 2D triangular meshes and graph-based sparse matrices. It reads mesh nodes and elements from files, derives node-to-element, node-neighbor, edge-to-element and element-neighbor mappings, builds directed and distance-weighted graphs, and assembles a sparse matrix in Compressed Row Storage (CRS) format with matrix-vector multiplication and ASCII/binary file I/O.

Binary matrix files store raw `std::size_t` and `double` representations and are intended for same-platform Linux use with a matching ABI/endianness; use ASCII I/O for portable exchange.

## Requirements

- CMake 3.28 or newer
- A C++20 compiler
- Optional: `clang-tidy` and `clang-format` 18 or newer for the developer tooling targets

## Project structure

```text
.
├── .clang-format
├── .clang-tidy
├── .github/workflows/tests.yml
├── CMakeLists.txt
├── LICENSE
├── README.md
├── cmake/matrices_meshes_femConfig.cmake.in
├── data/meshes/
│   ├── 3x3_coords_2d.txt
│   └── 3x3_elements_2d.txt
├── examples/main.cpp
├── include/matrices-meshes-fem/
│   ├── coord.h
│   ├── graph.h
│   ├── mesh.h
│   ├── sparsematrixcrs.h
│   └── weightedgraph.h
├── src/
│   ├── graph.cpp
│   ├── mesh.cpp
│   ├── sparsematrixcrs.cpp
│   └── weightedgraph.cpp
└── tests/
    ├── CMakeLists.txt
    ├── capture_stdout.h
    ├── test_coord.cpp
    ├── test_graph.cpp
    ├── test_mesh.cpp
    ├── test_sparsematrixcrs.cpp
    └── test_weightedgraph.cpp
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

`CMAKE_EXPORT_COMPILE_COMMANDS` is always enabled for editor and tooling integration. The shared warning set is applied only to project targets (the library, the example, and tests): GNU and Clang receive `-Wall`, `-Wextra`, `-Wpedantic`, `-Wconversion`, `-Wsign-conversion`, `-Weffc++`, `-Woverloaded-virtual`, `-Wfloat-equal`, `-Wshadow`, and `-Wredundant-decls`; MSVC receives `/W4`.

### CMake options

| Option | Default | Description |
|---|---:|---|
| `BUILD_TESTING` | `OFF` | Build the Catch2 test suite. |
| `ENABLE_CLANG_TIDY` | `OFF` | Run clang-tidy on project targets using `.clang-tidy`. |
| `ENABLE_CLANG_EVERYTHING` | `OFF` | Add Clang's `-Weverything` preset with project-specific suppressions. Clang only. |
| `ENABLE_FORMAT_TARGETS` | `OFF` | Add the `mmf-format` and `mmf-format-check` targets. |
| `ENABLE_FIRST_ERROR_ONLY` | `OFF` | Stop compiling after the first error (`-fmax-errors=1` on GNU, `-ferror-limit=1` on Clang). |
| `ENABLE_NATIVE_ARCH` | `OFF` | Add `-march=native`; no MSVC equivalent. |
| `ENABLE_FAST_MATH` | `OFF` | Add `-ffast-math` on GNU/Clang and `/fp:fast` on MSVC. |

## Developer tooling

Clang-tidy and clang-format are opt-in so they do not affect ordinary builds or fetched dependencies.

```bash
cmake -S . -B build \
    -DBUILD_TESTING=ON \
    -DENABLE_CLANG_TIDY=ON \
    -DENABLE_FORMAT_TARGETS=ON
cmake --build build --parallel

# Check formatting without modifying files:
cmake --build build --target mmf-format-check

# Format files in place:
cmake --build build --target mmf-format
```

- `ENABLE_CLANG_TIDY=ON` requires `clang-tidy` on `PATH` and the `.clang-tidy` configuration file. It is applied per project target, so fetched Catch2 sources are not analyzed.
- `ENABLE_FORMAT_TARGETS=ON` requires `clang-format` 18 or newer and the `.clang-format` configuration file.
- `ENABLE_CLANG_EVERYTHING=ON` is a Clang-only preset. It is ignored by GNU and MSVC.
- The GitHub Actions workflow currently only configures, builds, and runs the tests; the formatting and static-analysis targets are local developer tools.

## Tests

Tests are written with [Catch2](https://github.com/catchorg/Catch2) v3.16.1, pinned via `FetchContent`, and registered with CTest. The first configure requires network access to download Catch2. The suite covers `Coord` (`tests/test_coord.cpp`), `Mesh` (`tests/test_mesh.cpp`) using the sample mesh in `data/meshes/` plus temporary test files, `Graph` (`tests/test_graph.cpp`), `WeightedGraph` (`tests/test_weightedgraph.cpp`), and `SparseMatrixCRS` (`tests/test_sparsematrixcrs.cpp`). The shared RAII stdout-capture helper lives in `tests/capture_stdout.h`.

`BUILD_TESTING` defaults to `OFF`, so pass `-DBUILD_TESTING=ON` explicitly:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure --parallel
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
