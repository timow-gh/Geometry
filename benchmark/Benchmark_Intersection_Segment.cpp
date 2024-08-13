#include <Geometry/Intersect/IntersectSegment.hpp>
#include <Geometry/Segment.hpp>
#include <benchmark/benchmark.h>

#include <linal/vec.hpp>

using namespace Geometry;

static void BM_Intersection_Seg3dSeg3d(benchmark::State& state)
{
  Segment3d segA{linal::double3{}, linal::double3{2, 0, 0}};
  Segment3d segB{linal::double3{1, 0, 0}, linal::double3Y};

  for (auto _: state)
  {
    Segment3d intersection;
    auto res = Geometry::intersect(segA, segB);
    benchmark::DoNotOptimize(res);
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Intersection_Seg3dSeg3d);

static void BM_Intersection_Seg3fSeg3f(benchmark::State& state)
{
  Segment3f segA{linal::float3{}, linal::float3{2, 0, 0}};
  Segment3f segB{linal::float3{1, 0, 0}, linal::float3Y};

  for (auto _: state)
  {
    Segment3f intersection;
    auto res = Geometry::intersect(segA, segB);
    benchmark::DoNotOptimize(res);
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Intersection_Seg3fSeg3f);

static void BM_Intersection_Seg2dSeg2d(benchmark::State& state)
{
  Segment2d segA{linal::double2{}, linal::double2{2, 0}};
  Segment2d segB{linal::double2{1, 0}, linal::double2Y};

  for (auto _: state)
  {
    Segment2d intersection;
    auto res = Geometry::intersect(segA, segB);
    benchmark::DoNotOptimize(res);
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Intersection_Seg2dSeg2d);

static void BM_Intersection_Seg2fSeg2f(benchmark::State& state)
{
  Segment2f segA{linal::float2{}, linal::float2{2, 0}};
  Segment2f segB{linal::float2{1, 0}, linal::float2Y};

  for (auto _: state)
  {
    Segment2f intersection;
    auto res = Geometry::intersect(segA, segB);
    benchmark::DoNotOptimize(res);
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Intersection_Seg2fSeg2f);

// Run the benchmark
BENCHMARK_MAIN();
