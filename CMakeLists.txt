cmake_minimum_required(VERSION 3.28)

project(matrices_meshes_fem
    VERSION 0.1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)


# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
option(BUILD_TESTING "Enable testing" OFF)
option(ENABLE_CLANG_TIDY "Enable Clang-Tidy analysis" OFF)
option(ENABLE_FORMAT_TARGETS "Add clang-format targets" OFF)
option(ENABLE_NATIVE_ARCH "Use -march=native for CPU code" OFF)
option(ENABLE_FAST_MATH "Enable fast-math optimizations" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Build type (Debug, Release, RelWithDebInfo)")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo)
endif()

# The language standard is requested per target via target_compile_features().
set(CMAKE_CXX_EXTENSIONS OFF)


# -----------------------------------------------------------------------------
# Clang-Tidy
# -----------------------------------------------------------------------------
if(ENABLE_CLANG_TIDY)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy")
        message(FATAL_ERROR "ENABLE_CLANG_TIDY=ON but ${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy is missing")
    endif()

    find_program(CLANG_TIDY_EXE
        NAMES clang-tidy
        REQUIRED
    )
    mark_as_advanced(CLANG_TIDY_EXE)

    set(MMF_CLANG_TIDY
        "${CLANG_TIDY_EXE}"
        "--config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy"
    )

    message(STATUS "Clang-Tidy enabled: ${CLANG_TIDY_EXE}")
endif()


# -----------------------------------------------------------------------------
# Warning and optimization flags, shared by all targets
# -----------------------------------------------------------------------------
function(matrices_meshes_fem_set_compile_options target)
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wall>
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wextra>
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wpedantic>
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wconversion>
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wsign-conversion>
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4>
    )

    if(ENABLE_NATIVE_ARCH)
        # MSVC has no direct equivalent of -march=native.
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-march=native>
        )
    endif()

    if(ENABLE_FAST_MATH)
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-ffast-math>
            $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/fp:fast>
        )
    endif()

    # Apply Clang-Tidy only to project targets, not to fetched dependencies.
    if(ENABLE_CLANG_TIDY)
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${MMF_CLANG_TIDY}")
    endif()
endfunction()


# -----------------------------------------------------------------------------
# Clang-Format
# -----------------------------------------------------------------------------
if(ENABLE_FORMAT_TARGETS)
    find_program(CLANG_FORMAT_EXE
        NAMES clang-format
        REQUIRED
    )
    mark_as_advanced(CLANG_FORMAT_EXE)

    set(MMF_CLANG_FORMAT_MIN_VERSION 18 CACHE STRING "Minimum clang-format major version")
    execute_process(
        COMMAND ${CLANG_FORMAT_EXE} --version
        OUTPUT_VARIABLE MMF_CLANG_FORMAT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(MMF_CLANG_FORMAT_VERSION MATCHES "version ([0-9]+)")
        if(CMAKE_MATCH_1 LESS MMF_CLANG_FORMAT_MIN_VERSION)
            message(FATAL_ERROR
                "clang-format ${MMF_CLANG_FORMAT_MIN_VERSION} or newer is required "
                "(found: ${MMF_CLANG_FORMAT_VERSION})")
        endif()
    else()
        message(WARNING "Could not determine clang-format version; formatting may differ across versions.")
    endif()

    file(GLOB_RECURSE MMF_CXX_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h
    )

    add_custom_target(mmf-format
        COMMAND ${CLANG_FORMAT_EXE} -i ${MMF_CXX_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting C++ source files"
    )

    add_custom_target(mmf-format-check
        COMMAND ${CLANG_FORMAT_EXE}
        --dry-run
        --Werror
        ${MMF_CXX_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking C++ formatting"
    )
endif()


# -----------------------------------------------------------------------------
# matrices_meshes_fem
# -----------------------------------------------------------------------------
add_library(matrices_meshes_fem
    src/mesh.cpp
    src/sparsematrixcrs.cpp
    src/weightedgraph.cpp
    src/graph.cpp
)
add_library(matrices_meshes_fem::matrices_meshes_fem ALIAS matrices_meshes_fem)

target_include_directories(matrices_meshes_fem
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(matrices_meshes_fem PUBLIC cxx_std_20)

matrices_meshes_fem_set_compile_options(matrices_meshes_fem)


# -----------------------------------------------------------------------------
# Example
# -----------------------------------------------------------------------------
add_executable(example_main
    examples/main.cpp
)

target_link_libraries(example_main
    PRIVATE
    matrices_meshes_fem::matrices_meshes_fem
)

matrices_meshes_fem_set_compile_options(example_main)


# -----------------------------------------------------------------------------
# Install and package export
# -----------------------------------------------------------------------------
set(MMF_CMAKE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/matrices_meshes_fem)

install(TARGETS matrices_meshes_fem
    EXPORT matrices_meshes_femTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT matrices_meshes_femTargets
    FILE matrices_meshes_femTargets.cmake
    NAMESPACE matrices_meshes_fem::
    DESTINATION ${MMF_CMAKE_INSTALL_DIR}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/matrices_meshes_femConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/matrices_meshes_femConfig.cmake
    INSTALL_DESTINATION ${MMF_CMAKE_INSTALL_DIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/matrices_meshes_femConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/matrices_meshes_femConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/matrices_meshes_femConfigVersion.cmake
    DESTINATION ${MMF_CMAKE_INSTALL_DIR}
)


# Enable testing
include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
