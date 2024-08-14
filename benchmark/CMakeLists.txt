FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.6.0)

set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_EXCEPTIONS OFF CACHE BOOL "" FORCE)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(BENCHMARK_USE_LIBCXX ON CACHE BOOL "" FORCE)
else ()
    set(BENCHMARK_USE_LIBCXX OFF CACHE BOOL "" FORCE)
endif ()
FetchContent_MakeAvailable(benchmark)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(benchmark INTERFACE
            "-Wno-weak-vtables"
            )
endif ()

set(LIBS Geometry::Geometry benchmark::benchmark benchmark::benchmark_main)

function(create_geometry_benchmark benchMarkName)
    add_executable(${benchMarkName} ${benchMarkName}.cpp)
    target_link_libraries(${benchMarkName} PRIVATE "${LIBS}")
endfunction()

create_geometry_benchmark(Benchmark_Intersection_Plane)
create_geometry_benchmark(Benchmark_Intersection_Line)
create_geometry_benchmark(Benchmark_Intersection_Segment)
