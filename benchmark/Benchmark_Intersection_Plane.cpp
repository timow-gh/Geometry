#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <benchmark/benchmark.h>

#include "Geometry/Intersection/IntersectionPlane.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/Plane.hpp"
#include "Geometry/Ray.hpp"
#include "Geometry/Segment.hpp"

using namespace Geometry;
using namespace LinAl;

static void BM_Intersection_PlaneLine3d(benchmark::State &state) {
  Plane plane{ZERO_VEC3D, X_VEC3D};
  Line3d line{Vec3d{1, 1, 1}, X_VEC3D};
  for (auto _ : state) {
    auto intersection = calcIntersection(plane, line);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_PlaneLine3d);

static void BM_Intersection_PlaneRay3d(benchmark::State &state) {
  Plane plane{ZERO_VEC3D, X_VEC3D};
  Ray3d ray{Vec3d{1, 1, 1}, -X_VEC3D};
  for (auto _ : state) {
    auto intersection = calcIntersection(plane, ray);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_PlaneRay3d);

static void BM_Intersection_PlaneSegment3d(benchmark::State &state) {
  Plane plane{ZERO_VEC3D, X_VEC3D};
  Segment3d segment{Vec3d{-1, 1, 1}, X_VEC3D};
  for (auto _ : state) {
    auto intersection = calcIntersection(plane, segment);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_PlaneSegment3d);

// Run the benchmark
BENCHMARK_MAIN();
#pragma clang diagnostic pop