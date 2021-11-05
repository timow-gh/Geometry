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

add_executable(Test_Sphere Test_Sphere.cpp)
target_link_libraries(Test_Sphere PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Test_Sphere COMMAND Test_Sphere)

add_executable(Test_Ray Test_Ray.cpp)
target_link_libraries(Test_Ray PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Test_Ray COMMAND Test_Ray)

add_executable(Test_Cylinder Test_Cylinder.cpp)
target_link_libraries(Test_Cylinder PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Test_Cylinder COMMAND Test_Cylinder)

add_executable(Test_Lcs Test_Lcs.cpp)
target_link_libraries(Test_Lcs PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Test_Lcs COMMAND Test_Lcs)

add_executable(Test_VecOperations_FIXME Test_VecOperations.cpp)
target_link_libraries(Test_VecOperations_FIXME PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Test_VecOperations_FIXME COMMAND Test_VecOperations_FIXME)

add_executable(Test_PolyLine Test_PolyLine.cpp)
target_link_libraries(Test_PolyLine PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Test_PolyLine COMMAND Test_PolyLine)

add_executable(Test_Cone Test_Cone.cpp)
target_link_libraries(Test_Cone PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Test_Cone COMMAND Test_Cone)
