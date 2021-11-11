FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.6.0)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
FetchContent_MakeAvailable(benchmark)

set(LIBS Core::Types Geometry::Geometry benchmark::benchmark benchmark::benchmark_main)

if (NOT MSVC)
    add_executable(Benchmark_LinAl LinAl.cpp)
    target_link_libraries(Benchmark_LinAl PRIVATE "${LIBS}")
endif ()

function(create_geometry_benchmark benchMarkName)
    add_executable(${benchMarkName} ${benchMarkName}.cpp)
    target_link_libraries(${benchMarkName} PRIVATE "${LIBS}")
endfunction()

create_geometry_benchmark(Benchmark_Intersection_Plane)
create_geometry_benchmark(Benchmark_Intersection_Line)
create_geometry_benchmark(Benchmark_Intersection_Segment)
