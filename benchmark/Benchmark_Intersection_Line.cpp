#if !defined(_MSVC_LANG)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
#endif

#include <Geometry/Intersection/IntersectionLine.hpp>
#include <Geometry/Line.hpp>
#include <benchmark/benchmark.h>
#include <linal/vec2.hpp>
#include <linal/vec3.hpp>

using namespace Geometry;

static void BM_Intersection_Line3dLine3d(benchmark::State& state)
{
  Line3d lineA{linal::ZERO_VEC3D, linal::X_VEC3D};
  Line3d lineB{linal::vec3d{1, 0, 0}, linal::Y_VEC3D};

  linal::vec3d intersection;
  uint32_t res;
  for (auto _: state)
  {
    res = Geometry::intersection(lineA, lineB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Line3dLine3d);

static void BM_Intersection_Line3fLine3f(benchmark::State& state)
{
  Line3f lineA{linal::ZERO_VEC3F, linal::X_VEC3F};
  Line3f lineB{linal::vec3f{1, 0, 0}, linal::Y_VEC3F};

  for (auto _: state)
  {
    linal::vec3f intersection;
    uint32_t res = Geometry::intersection(lineA, lineB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Line3fLine3f);

static void BM_Intersection_Line2dLine2d(benchmark::State& state)
{
  Line2d lineA{linal::ZERO_VEC2D, linal::X_VEC2D};
  Line2d lineB{linal::vec2d{1, 0}, linal::Y_VEC2D};

  for (auto _: state)
  {
    linal::vec2d intersection;
    uint32_t res = Geometry::intersection(lineA, lineB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Line2dLine2d);

static void BM_Intersection_Line2fLine2f(benchmark::State& state)
{
  Line2f lineA{linal::ZERO_VEC2F, linal::X_VEC2F};
  Line2f lineB{linal::vec2f{1, 0}, linal::Y_VEC2F};

  for (auto _: state)
  {
    linal::vec2f intersection;
    uint32_t res = Geometry::intersection(lineA, lineB, intersection);
    benchmark::DoNotOptimize(res);
    benchmark::DoNotOptimize(intersection);
  }
}
BENCHMARK(BM_Intersection_Line2fLine2f);

// Run the benchmark
BENCHMARK_MAIN();

#if !defined(_MSVC_LANG)
#pragma GCC diagnostic pop
#endif
