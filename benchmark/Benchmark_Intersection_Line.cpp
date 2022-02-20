

#include <benchmark/benchmark.h>

#include <Geometry/Intersection/IntersectionLine.hpp>
#include <Geometry/Line.hpp>

using namespace Geometry;
using namespace LinAl;

static void BM_Intersection_Line3dLine3d(benchmark::State &state) {
  Line3d lineA{ZERO_VEC3D, X_VEC3D};
  Line3d lineB{Vec3d{1, 0, 0}, Y_VEC3D};

  for (auto _ : state) {
    LinAl::Vec3d intersection;
    uint32_t res = calcIntersection(lineA, lineB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Line3dLine3d);

static void BM_Intersection_Line3fLine3f(benchmark::State &state) {
  Line3f lineA{ZERO_VEC3F, X_VEC3F};
  Line3f lineB{Vec3f{1, 0, 0}, Y_VEC3F};

  for (auto _ : state) {
    LinAl::Vec3f intersection;
    uint32_t res = calcIntersection(lineA, lineB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Line3fLine3f);

static void BM_Intersection_Line2dLine2d(benchmark::State &state) {
  Line2d lineA{ZERO_VEC2D, X_VEC2D};
  Line2d lineB{Vec2d{1, 0}, Y_VEC2D};

  for (auto _ : state) {
    LinAl::Vec2d intersection;
    uint32_t res = calcIntersection(lineA, lineB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Line2dLine2d);

static void BM_Intersection_Line2fLine2f(benchmark::State &state) {
  Line2f lineA{ZERO_VEC2F, X_VEC2F};
  Line2f lineB{Vec2f{1, 0}, Y_VEC2F};

  for (auto _ : state) {
    LinAl::Vec2f intersection;
    uint32_t res = calcIntersection(lineA, lineB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Line2fLine2f);

// Run the benchmark
BENCHMARK_MAIN();
