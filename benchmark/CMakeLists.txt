FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.6.0)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
FetchContent_MakeAvailable(benchmark)

set(LIBS Core::Types Geometry::Geometry benchmark::benchmark benchmark::benchmark_main)

add_executable(Benchmark_LinAl LinAl.cpp)
target_link_libraries(Benchmark_LinAl PRIVATE "${LIBS}")

add_executable(Benchmark_Intersection_Line Benchmark_Intersection_Line.cpp)
target_link_libraries(Benchmark_Intersection_Line PRIVATE  "${LIBS}")

add_executable(Benchmark_Intersection_Segment Benchmark_Intersection_Segment.cpp)
target_link_libraries(Benchmark_Intersection_Segment PRIVATE "${LIBS}")

add_executable(Benchmark_Intersection_Plane Benchmark_Intersection_Plane.cpp)
target_link_libraries(Benchmark_Intersection_Plane PRIVATE "${LIBS}")
