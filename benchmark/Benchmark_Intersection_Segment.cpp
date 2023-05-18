#include <Geometry/Intersection/IntersectionSegment.hpp>
#include <Geometry/Segment.hpp>
#include <benchmark/benchmark.h>
#include <linal/vec2.hpp>
#include <linal/vec3.hpp>

using namespace Geometry;

static void BM_Intersection_Seg3dSeg3d(benchmark::State& state)
{
  Segment3d segA{linal::ZERO_VEC3D, linal::vec3d{2, 0, 0}};
  Segment3d segB{linal::vec3d{1, 0, 0}, linal::Y_VEC3D};

  for (auto _: state)
  {
    Segment3d intersection;
    uint32_t res = Geometry::intersection(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg3dSeg3d);

static void BM_Intersection_Seg3fSeg3f(benchmark::State& state)
{
  Segment3f segA{linal::ZERO_VEC3F, linal::vec3f{2, 0, 0}};
  Segment3f segB{linal::vec3f{1, 0, 0}, linal::Y_VEC3F};

  for (auto _: state)
  {
    Segment3f intersection;
    uint32_t res = Geometry::intersection(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg3fSeg3f);

static void BM_Intersection_Seg2dSeg2d(benchmark::State& state)
{
  Segment2d segA{linal::ZERO_VEC2D, linal::vec2d{2, 0}};
  Segment2d segB{linal::vec2d{1, 0}, linal::Y_VEC2D};

  for (auto _: state)
  {
    Segment2d intersection;
    uint32_t res = Geometry::intersection(segA, segB, intersection);
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
    uint32_t res = Geometry::intersection(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg2fSeg2f);

// Run the benchmark
BENCHMARK_MAIN();
