#include <Geometry/Intersection/IntersectionPlane.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Ray.hpp>
#include <Geometry/Segment.hpp>
#include <benchmark/benchmark.h>
#include <linal/Vec3.hpp>

using namespace Geometry;

static void BM_Intersection_PlaneLine3d(benchmark::State& state)
{
  Plane plane{linal::ZERO_VEC3D, linal::X_VEC3D};
  Line3d line{linal::Vec3d{1, 1, 1}, linal::X_VEC3D};
  for (auto _: state)
  {
    auto intersection = Geometry::intersection(plane, line);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_PlaneLine3d);

static void BM_Intersection_PlaneRay3d(benchmark::State& state)
{
  Plane plane{linal::ZERO_VEC3D, linal::X_VEC3D};
  Ray3d ray{linal::Vec3d{1, 1, 1}, -linal::X_VEC3D};
  for (auto _: state)
  {
    auto intersection = Geometry::intersection(plane, ray);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_PlaneRay3d);

static void BM_Intersection_PlaneSegment3d(benchmark::State& state)
{
  Plane plane{linal::ZERO_VEC3D, linal::X_VEC3D};
  Segment3d segment{linal::Vec3d{-1, 1, 1}, linal::X_VEC3D};
  for (auto _: state)
  {
    auto intersection = Geometry::intersection(plane, segment);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_PlaneSegment3d);

// Run the benchmark
BENCHMARK_MAIN();
