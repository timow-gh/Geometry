find_package(benchmark CONFIG REQUIRED)

set(LIBS Core::Types LinAl::LinAl Geometry::Geometry benchmark::benchmark benchmark::benchmark_main)

add_executable(Benchmark_LinAl LinAl.cpp)
target_link_libraries(Benchmark_LinAl PRIVATE "${LIBS}")

add_executable(Benchmark_Intersection_Line Benchmark_Intersection_Line.cpp)
target_link_libraries(Benchmark_Intersection_Line PRIVATE  "${LIBS}")

add_executable(Benchmark_Intersection_Segment Benchmark_Intersection_Segment.cpp)
target_link_libraries(Benchmark_Intersection_Segment PRIVATE "${LIBS}")

add_executable(Benchmark_Intersection_Plane Benchmark_Intersection_Plane.cpp)
target_link_libraries(Benchmark_Intersection_Plane PRIVATE "${LIBS}")
