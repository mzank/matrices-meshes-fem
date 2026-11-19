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

