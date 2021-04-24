cmake_minimum_required(VERSION 3.16)
project(Tests
        LANGUAGES CXX)

option(INSTALL_GTEST OFF)
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.10.0
)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(Distance)
add_subdirectory(Intersection)
add_subdirectory(MeshBuilder)

set(TEST_GEOMETRY_SOURCES
    Test_Ray.cpp
    Test_VecOperations.cpp
    Test_Sphere.cpp
    Test_Cylinder.cpp
    Test_Lcs.cpp
    )

add_executable(test_geometry ${TEST_GEOMETRY_SOURCES})
target_link_libraries(test_geometry
                      PRIVATE
                      gtest_main
                      Geometry::Geometry
                      )
add_test(NAME test_geometry COMMAND test_geometry)

