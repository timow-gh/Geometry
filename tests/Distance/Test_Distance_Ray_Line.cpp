#include <Geometry/Distance/DistanceRay.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Ray.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>
#include <vector>

using namespace Geometry;

class Distance_Ray_Line_Test : public ::testing::Test {
protected:
  linal::double3 m_source{1, 1, 1};
  Ray3d m_ray{m_source, linal::double3X};
};

TEST_F(Distance_Ray_Line_Test, Parallel)
{
  Line3d line{linal::double3{0, 0, 0}, linal::double3X};
  double dist = Geometry::distance(line, m_ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Distance_Ray_Line_Test, Collinear)
{
  Line3d line{linal::double3{0, 0, 0}, linal::double3X};
  Ray3d ray{linal::double3{1, 0, 0}, linal::double3X};
  double dist = Geometry::distance(line, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Ray_Line_Test, SameSourcePointWith)
{
  Line3d line{linal::double3{1, 1, 1}, linal::double3Y};
  double dist = Geometry::distance(line, m_ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);

  dist = Geometry::distance(m_ray, line);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Ray_Line_Test, SameSourcePointWithParallelDirection)
{
  Line3d line{linal::double3{1, 1, 1}, linal::double3X};
  double dist = Geometry::distance(line, m_ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Ray_Line_Test, DifferentSources)
{
  Line3d line{linal::double3{1, 1, 0}, linal::double3Y};
  double dist = Geometry::distance(line, m_ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Distance_Ray_Line_Test, DifferentSourcesWithParallelDirection)
{
  Line3d line{linal::double3{1, 1, 0}, linal::double3X};
  double dist = Geometry::distance(line, m_ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Distance_Ray_Line_Test, RaySourceIsClosestToLine)
{
  Line3d line{linal::double3{0, 0, 0}, linal::double3Y};
  Ray3d ray{linal::double3{1, 0, 0}, linal::double3X};
  double dist = Geometry::distance(line, ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Distance_Ray_Line_Test, SkewLines)
{
  Line3d line{linal::double3{0, 0, 0}, linal::double3Y};
  Ray3d ray{linal::double3{0, 1, 1}, linal::double3X};
  double dist = Geometry::distance(line, ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}
