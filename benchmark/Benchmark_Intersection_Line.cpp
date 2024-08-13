#if !defined(_MSVC_LANG)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
#endif

#include <Geometry/Intersect/IntersectLine.hpp>
#include <Geometry/Line.hpp>
#include <benchmark/benchmark.h>
#include <linal/vec.hpp>

using namespace Geometry;

static void BM_Intersection_Line3dLine3d(benchmark::State& state)
{
  Line3d lineA{linal::double3{}, linal::double3X};
  Line3d lineB{linal::double3{1, 0, 0}, linal::double3Y};

  for (auto _: state)
  {
    auto res = Geometry::intersect(lineA, lineB);
    benchmark::DoNotOptimize(res);
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Intersection_Line3dLine3d);

static void BM_Intersection_Line3fLine3f(benchmark::State& state)
{
  Line3f lineA{linal::float3{}, linal::float3X};
  Line3f lineB{linal::float3{1, 0, 0}, linal::float3Y};

  for (auto _: state)
  {
    auto res = Geometry::intersect(lineA, lineB);
    benchmark::DoNotOptimize(res);
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Intersection_Line3fLine3f);

static void BM_Intersection_Line2dLine2d(benchmark::State& state)
{
  Line2d lineA{linal::double2{}, linal::double2X};
  Line2d lineB{linal::double2{1, 0}, linal::double2Y};

  for (auto _: state)
  {
    auto res = Geometry::intersect(lineA, lineB);
    benchmark::DoNotOptimize(res);
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Intersection_Line2dLine2d);

static void BM_Intersection_Line2fLine2f(benchmark::State& state)
{
  Line2f lineA{linal::float2{}, linal::float2X};
  Line2f lineB{linal::float2{1, 0}, linal::float2Y};

  for (auto _: state)
  {
    auto res = Geometry::intersect(lineA, lineB);
    benchmark::DoNotOptimize(res);
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Intersection_Line2fLine2f);

// Run the benchmark
BENCHMARK_MAIN();

#if !defined(_MSVC_LANG)
#pragma GCC diagnostic pop
#endif
