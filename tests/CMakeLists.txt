# =====================================================
# Catch2
# =====================================================

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG 08092139210881f01a34e60c5f2fd6afa0ea4024 # v3.16.1
        SYSTEM
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(Catch2)

include(Catch)

# =====================================================
# C++ tests
# =====================================================

add_executable(test_coord
        test_coord.cpp
)
target_link_libraries(test_coord
        PRIVATE
        matrices_meshes_fem
        Catch2::Catch2WithMain
)
catch_discover_tests(test_coord)


add_executable(test_mesh
        test_mesh.cpp
)
target_link_libraries(test_mesh
        PRIVATE
        matrices_meshes_fem
        Catch2::Catch2WithMain
)
target_compile_definitions(test_mesh
        PRIVATE
        MMF_TEST_DATA_DIR="${PROJECT_SOURCE_DIR}/data/meshes"
)
catch_discover_tests(test_mesh)


add_executable(test_graph
        test_graph.cpp
)
target_link_libraries(test_graph
        PRIVATE
        matrices_meshes_fem
        Catch2::Catch2WithMain
)
catch_discover_tests(test_graph)


add_executable(test_weightedgraph
        test_weightedgraph.cpp
)
target_link_libraries(test_weightedgraph
        PRIVATE
        matrices_meshes_fem
        Catch2::Catch2WithMain
)
catch_discover_tests(test_weightedgraph)


add_executable(test_sparsematrixcrs
        test_sparsematrixcrs.cpp
)
target_link_libraries(test_sparsematrixcrs
        PRIVATE
        matrices_meshes_fem
        Catch2::Catch2WithMain
)
catch_discover_tests(test_sparsematrixcrs)

