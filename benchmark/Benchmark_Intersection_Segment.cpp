#include <Geometry/Intersect/IntersectSegment.hpp>
#include <Geometry/Segment.hpp>
#include <benchmark/benchmark.h>

#include <linal/vec.hpp>

using namespace Geometry;

static void BM_Intersection_Seg3dSeg3d(benchmark::State& state)
{
  Segment3d segA{linal::double3{}, linal::double3{2, 0, 0}};
  Segment3d segB{linal::double3{1, 0, 0}, double3Y};

  for (auto _: state)
  {
    Segment3d intersection;
    uint32_t res = Geometry::intersect(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg3dSeg3d);

static void BM_Intersection_Seg3fSeg3f(benchmark::State& state)
{
  Segment3f segA{linal::ZERO_VEC3F, linal::float3{2, 0, 0}};
  Segment3f segB{linal::float3{1, 0, 0}, linal::Y_VEC3F};

  for (auto _: state)
  {
    Segment3f intersection;
    uint32_t res = Geometry::intersect(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg3fSeg3f);

static void BM_Intersection_Seg2dSeg2d(benchmark::State& state)
{
  Segment2d segA{linal::ZERO_double2, linal::double2{2, 0}};
  Segment2d segB{linal::double2{1, 0}, linal::double2Y};

  for (auto _: state)
  {
    Segment2d intersection;
    uint32_t res = Geometry::intersect(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg2dSeg2d);

static void BM_Intersection_Seg2fSeg2f(benchmark::State& state)
{
  Segment2f segA{linal::ZERO_VEC2F, linal::vec2f{2, 0}};
  Segment2f segB{linal::vec2f{1, 0}, linal::Y_VEC2F};

  for (auto _: state)
  {
    Segment2f intersection;
    uint32_t res = Geometry::intersect(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg2fSeg2f);

// Run the benchmark
BENCHMARK_MAIN();
