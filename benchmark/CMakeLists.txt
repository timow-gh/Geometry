cmake_minimum_required(VERSION 3.16)
project(Benchmarks)

set(CMAKE_CXX_STANDARD 17)

find_package(benchmark CONFIG REQUIRED)

set(LIBS benchmark::benchmark Core::Core LinAl::LinAl Geometry::Geometry)

add_executable(Benchmark_LinAl LinAl.cpp)
target_link_libraries(Benchmark_LinAl "${LIBS}")

add_executable(Benchmark_Intersection_Line Benchmark_Intersection_Line.cpp)
target_link_libraries(Benchmark_Intersection_Line "${LIBS}")

add_executable(Benchmark_Intersection_Segment Benchmark_Intersection_Segment.cpp)
target_link_libraries(Benchmark_Intersection_Segment "${LIBS}")

add_executable(Benchmark_Intersection_Plane Benchmark_Intersection_Plane.cpp)
target_link_libraries(Benchmark_Intersection_Plane "${LIBS}")
