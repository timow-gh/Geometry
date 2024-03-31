#include <Geometry/Intersect/IntersectPlane.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Ray.hpp>
#include <Geometry/Segment.hpp>
#include <benchmark/benchmark.h>
#include <linal/vec.hpp>

using namespace Geometry;

static void BM_Intersection_PlaneLine3d(benchmark::State& state)
{
  Plane plane{linal::double3{}, linal::double3X};
  Line3d line{linal::double3{1, 1, 1}, linal::double3X};
  for (auto _: state)
  {
    auto intersection = Geometry::intersect(plane, line);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_PlaneLine3d);

static void BM_Intersection_PlaneRay3d(benchmark::State& state)
{
  Plane plane{linal::double3{}, linal::double3X};
  Ray3d ray{linal::double3{1, 1, 1}, -linal::double3X};
  for (auto _: state)
  {
    auto intersection = Geometry::intersect(plane, ray);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_PlaneRay3d);

static void BM_Intersection_PlaneSegment3d(benchmark::State& state)
{
  Plane plane{linal::double3{}, linal::double3X};
  Segment3d segment{linal::double3{-1, 1, 1}, linal::double3X};
  for (auto _: state)
  {
    auto intersection = Geometry::intersect(plane, segment);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_PlaneSegment3d);

// Run the benchmark
BENCHMARK_MAIN();
