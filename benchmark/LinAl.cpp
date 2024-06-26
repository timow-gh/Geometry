#include <benchmark/benchmark.h>
#include <linal/vec.hpp>
#include <linal/vec_operations.hpp>

static void BM_Vector3dAdd(benchmark::State& state)
{
  linal::double3 lhs(0.0);
  linal::double3 rhs(1.0);
  [[maybe_unused]] linal::double3 res;
  for (auto _: state)
  {
    res = lhs + rhs;
  }
}

static void BM_Vector3fAdd(benchmark::State& state)
{
  linal::float3 lhs(0.0f);
  linal::float3 rhs(1.0f);
  [[maybe_unused]] linal::float3 res;
  for (auto _: state)
  {
    res = lhs + rhs;
  }
}

static void BM_Vector3dDot(benchmark::State& state)
{
  linal::double3 lhs(0.0);
  linal::double3 rhs(1.0);
  [[maybe_unused]] double res = 0.0;
  for (auto _: state)
  {
    res = linal::dot(lhs, rhs);
  }
}

static void BM_Vector3fDot(benchmark::State& state)
{
  linal::float3 lhs(0.0f);
  linal::float3 rhs(1.0f);
  float_t res = 0.0f;
  for (auto _: state)
  {
    res = linal::dot(lhs, rhs);
  }
}

// Register the function as a benchmark
BENCHMARK(BM_Vector3dAdd);
BENCHMARK(BM_Vector3fAdd);
BENCHMARK(BM_Vector3dDot);
BENCHMARK(BM_Vector3fDot);

static void escape(void* p)
{
  // volatile to prevent compiler optimization of the assembly
  // ""   :    :  "g"(p)  : "memory"
  // inline assembly
  // comma seperated outputs
  // comma seperated list of intputs
  // clobbers
  asm volatile("" : : "g"(p) : "memory");
}

// "Reads" and "writes" the memory of the entire system
[[maybe_unused]] static void clobber()
{
  asm volatile("" : : : "memory");
}

static void BM_blaze_vec3i_add_unaligned_unpadded(benchmark::State& state)
{
  blaze::StaticVector<int64_t, 3UL, blaze::columnVector, blaze::unaligned, blaze::unpadded> a{1, 2, 3};
  blaze::StaticVector<int64_t, 3UL, blaze::columnVector, blaze::unaligned, blaze::unpadded> b{1, 2, 3};
  blaze::StaticVector<int64_t, 3UL, blaze::columnVector, blaze::unaligned, blaze::unpadded> c{1, 2, 3};
  for (auto _: state)
  {
    c = a + b;
    escape(&c);
  }
}

static void BM_blaze_vec3i_add(benchmark::State& state)
{
  blaze::StaticVector<int64_t, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> a{1, 2, 3};
  blaze::StaticVector<int64_t, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> b{1, 2, 3};
  blaze::StaticVector<int64_t, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> c{1, 2, 3};
  for (auto _: state)
  {
    c = a + b;
    escape(&c);
  }
}

#ifdef EIGEN_BENCHMARKS
static void BM_eigen3_vec3i_add(benchmark::State& state)
{
  Eigen::Vector3i a(1, 2, 3);
  Eigen::Vector3i b(1, 2, 3);
  Eigen::Vector3i c(1, 2, 3);
  for (auto _: state)
  {
    c = a + b;
    escape(&c);
  }
}
#endif

static void BM_blaze_vec3d_add(benchmark::State& state)
{
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> a{1, 2, 3};
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> b{1, 2, 3};
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> c{1, 2, 3};
  for (auto _: state)
  {
    c = a + b;
    escape(&c);
  }
}

#ifdef EIGEN_BENCHMARKS
static void BM_eigen3_vec3d_add(benchmark::State& state)
{
  Eigen::Vector3d a(1, 2, 3);
  Eigen::Vector3d b(1, 2, 3);
  Eigen::Vector3d c(1, 2, 3);
  for (auto _: state)
  {
    c = a + b;
    escape(&c);
  }
}
#endif

static double c{0.0};

static void BM_blaze_vec3d_dot(benchmark::State& state)
{
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> a{1, 2, 3};
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> b{1, 2, 3};
  for (auto _: state)
  {
    c = blaze::dot(a, b);
    escape(&c);
  }
}

#ifdef EIGEN_BENCHMARKS
static void BM_eigen3_vec3d_dot(benchmark::State& state)
{
  Eigen::Vector3d a(1, 2, 3);
  Eigen::Vector3d b(1, 2, 3);
  for (auto _: state)
  {
    c = a.dot(b);
    escape(&c);
  }
}
#endif

static void BM_linearalgebra_vec3d_dot(benchmark::State& state)
{
  linal::double3 a{1, 2, 3};
  linal::double3 b{1, 2, 3};
  for (auto _: state)
  {
    c = linal::dot(a, b);
    escape(&c);
  }
}

static void BM_blaze_vec3d_cross(benchmark::State& state)
{
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> a{1, 2, 3};
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> b{1, 2, 3};
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> d{};
  for (auto _: state)
  {
    d = blaze::cross(a, b);
    escape(&d);
  }
}

static void BM_blaze_vec3f_cross(benchmark::State& state)
{
  blaze::StaticVector<float_t, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> a{1, 2, 3};
  blaze::StaticVector<float_t, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> b{1, 2, 3};
  blaze::StaticVector<float_t, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> d{};
  for (auto _: state)
  {
    d = blaze::cross(a, b);
    escape(&d);
  }
}

#ifdef EIGEN_BENCHMARKS
static void BM_eigen3_vec3d_cross(benchmark::State& state)
{
  Eigen::Vector3d a(1, 2, 3);
  Eigen::Vector3d b(1, 2, 3);
  Eigen::Vector3d d;
  for (auto _: state)
  {
    d = a.cross(b);
    escape(&d);
  }
}
#endif

static void BM_linearalgebra_vec3d_cross(benchmark::State& state)
{
  linal::double3 a{1, 2, 3};
  linal::double3 b{1, 2, 3};
  linal::double3 d;
  for (auto _: state)
  {
    d = linal::cross(a, b);
    escape(&d);
  }
}

static void BM_blaze_vec3f_normalize(benchmark::State& state)
{
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> a{1, 2, 3};
  blaze::StaticVector<double, 3UL, blaze::columnVector, blaze::aligned, blaze::padded> b{1, 2, 3};
  for (auto _: state)
  {
    b = blaze::normalize(a);
    escape(&b);
  }
}

#ifdef EIGEN_BENCHMARKS
static void BM_eigen3_vec3d_normalize(benchmark::State& state)
{
  Eigen::Vector3d a(1, 2, 3);
  Eigen::Vector3d b(1, 2, 3);
  for (auto _: state)
  {
    b = a.normalized();
    escape(&b);
  }
}
#endif

static void BM_linearalgebra_vec3d_projection(benchmark::State& state)
{
  linal::double3 a{1, 1, 1};
  linal::double3 b{1, 1, 0};
  linal::double3 d;
  for (auto _: state)
  {
    d = linal::projection(a, b);
    escape(&d);
  }
}

static void BM_linearalgebra_vec3d_rejection(benchmark::State& state)
{
  linal::double3 a{1, 1, 1};
  linal::double3 b{1, 1, 0};
  linal::double3 d;
  for (auto _: state)
  {
    d = linal::rejection(a, b);
    escape(&d);
  }
}

BENCHMARK(BM_blaze_vec3i_add_unaligned_unpadded);
BENCHMARK(BM_blaze_vec3i_add);

BENCHMARK(BM_blaze_vec3d_add);

BENCHMARK(BM_blaze_vec3d_dot);
BENCHMARK(BM_linearalgebra_vec3d_dot);

BENCHMARK(BM_blaze_vec3d_cross);
BENCHMARK(BM_blaze_vec3f_cross);
BENCHMARK(BM_linearalgebra_vec3d_cross);

BENCHMARK(BM_blaze_vec3f_normalize);

BENCHMARK(BM_linearalgebra_vec3d_projection);
BENCHMARK(BM_linearalgebra_vec3d_rejection);

#ifdef EIGEN_BENCHMARKS
BENCHMARK(BM_eigen3_vec3i_add);
BENCHMARK(BM_eigen3_vec3d_add);
BENCHMARK(BM_eigen3_vec3d_dot);
BENCHMARK(BM_eigen3_vec3d_cross);
BENCHMARK(BM_eigen3_vec3d_normalize);
#endif

// Run the benchmark
BENCHMARK_MAIN();
