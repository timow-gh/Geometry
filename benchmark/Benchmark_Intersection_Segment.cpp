#include <benchmark/benchmark.h>

#include <Geometry/Intersection/IntersectionSegment.hpp>
#include <Geometry/Segment.hpp>

using namespace Geometry;
using namespace LinAl;

static void BM_Intersection_Seg3dSeg3d(benchmark::State &state) {
  Segment3d segA{ZERO_VEC3D, Vec3d{2, 0, 0}};
  Segment3d segB{Vec3d{1, 0, 0}, Y_VEC3D};

  for (auto _ : state) {
    Segment3d intersection;
    uint32_t res = calcIntersection(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg3dSeg3d);

static void BM_Intersection_Seg3fSeg3f(benchmark::State &state) {
  Segment3f segA{ZERO_VEC3F, Vec3f{2, 0, 0}};
  Segment3f segB{Vec3f{1, 0, 0}, Y_VEC3F};

  for (auto _ : state) {
    Segment3f intersection;
    uint32_t res = calcIntersection(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg3fSeg3f);

static void BM_Intersection_Seg2dSeg2d(benchmark::State &state) {
  Segment2d segA{ZERO_VEC2D, Vec2d{2, 0}};
  Segment2d segB{Vec2d{1, 0}, Y_VEC2D};

  for (auto _ : state) {
    Segment2d intersection;
    uint32_t res = calcIntersection(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg2dSeg2d);

static void BM_Intersection_Seg2fSeg2f(benchmark::State &state) {
  Segment2f segA{ZERO_VEC2F, Vec2f{2, 0}};
  Segment2f segB{Vec2f{1, 0}, Y_VEC2F};

  for (auto _ : state) {
    Segment2f intersection;
    uint32_t res = calcIntersection(segA, segB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Seg2fSeg2f);

// Run the benchmark
BENCHMARK_MAIN();
