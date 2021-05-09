option(INSTALL_GTEST OFF)
mark_as_advanced(BUILD_GMOCK)
mark_as_advanced(BUILD_TESTING)
mark_as_advanced(INSTALL_GTEST)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.10.0
)
FetchContent_MakeAvailable(googletest)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

mark_as_advanced(FETCHCONTENT_BASE_DIR)
mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_QUIET)
mark_as_advanced(FETCHCONTENT_SOURCE_DIR_GOOGLETEST)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED_GOOGLETEST)

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

