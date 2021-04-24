add_executable(Distance_Line Test_Distance_Line.cpp)
target_link_libraries(Distance_Line PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Distance_Line COMMAND Distance_Line)

add_executable(Distance_Ray Test_Distance_Ray.cpp)
target_link_libraries(Distance_Ray PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Distance_Ray COMMAND Distance_Ray)

add_executable(Distance_Plane Test_Distance_Plane.cpp)
target_link_libraries(Distance_Plane PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Distance_Plane COMMAND Distance_Plane)

add_executable(Distance_Segment Test_Distance_Segment.cpp)
target_link_libraries(Distance_Segment PRIVATE gtest_main Geometry::Geometry)
add_test(NAME Distance_Segment COMMAND Distance_Segment)