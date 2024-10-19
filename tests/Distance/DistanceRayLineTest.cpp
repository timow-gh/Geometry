#include <Geometry/Distance/DistanceRay.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Ray.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>
#include <vector>

using namespace Geometry;

class DistanceRayLineTest : public ::testing::Test {
protected:
  Ray3d m_ray{linal::double3{1, 1, 1}, linal::double3X};
};

TEST_F(DistanceRayLineTest, Parallel)
{
  Ray3d ray{linal::double3{0, 0, 2}, linal::double3X};
  Line3d line{linal::double3{0, 0, 0}, linal::double3X};
  double dist = Geometry::distance(line, ray);
  EXPECT_DOUBLE_EQ(dist, 2.0);
}

TEST_F(DistanceRayLineTest, Collinear)
{
  Line3d line{linal::double3{0, 0, 0}, linal::double3X};
  Ray3d ray{linal::double3{1, 0, 0}, linal::double3X};
  double dist = Geometry::distance(line, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(DistanceRayLineTest, SameSourcePointWith)
{
  Line3d line{linal::double3{1, 1, 1}, linal::double3Y};
  double dist = Geometry::distance(line, m_ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);

  dist = Geometry::distance(m_ray, line);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(DistanceRayLineTest, SameSourcePointWithParallelDirection)
{
  Line3d line{linal::double3{1, 1, 1}, linal::double3X};
  double dist = Geometry::distance(line, m_ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(DistanceRayLineTest, DifferentSources)
{
  Line3d line{linal::double3{1, 1, 0}, linal::double3Y};
  double dist = Geometry::distance(line, m_ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(DistanceRayLineTest, DifferentSourcesWithParallelDirection)
{
  Line3d line{linal::double3{1, 1, 0}, linal::double3X};
  double dist = Geometry::distance(line, m_ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(DistanceRayLineTest, RaySourceIsClosestToLine)
{
  Line3d line{linal::double3{0, 0, 0}, linal::double3Y};
  Ray3d ray{linal::double3{1, 0, 0}, linal::double3X};
  double dist = Geometry::distance(line, ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(DistanceRayLineTest, SkewLines)
{
  Line3d line{linal::double3{0, 0, 0}, linal::double3Y};
  Ray3d ray{linal::double3{0, 1, 1}, linal::double3X};
  double dist = Geometry::distance(line, ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}
