#include <Geometry/Distance/DistanceRay.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Ray.hpp>
#include <gtest/gtest.h>
#include <linal/vec3.hpp>
#include <vector>

using namespace Geometry;

class Distance_Ray_Test : public ::testing::Test {
protected:
  linal::vec3d source{1, 1, 1};
  Ray3d ray{source, linal::X_VEC3D};
};

TEST_F(Distance_Ray_Test, PointAtRayStart)
{
  double dist = Geometry::distance(ray, source);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Ray_Test, PointAtRayStart_ArgOrder)
{
  double dist = distance(source, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST(Distance_Ray, OnLine_UnitDist)
{
  linal::vec3d source{1, 0, 0};
  Ray3d ray{source, linal::X_VEC3D};
  double dist = Geometry::distance(ray, linal::ZERO_VEC3D);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST(Distance_Ray, OnLine_ZeroDist)
{
  linal::vec3d source{1, 0, 0};
  Ray3d ray{source, linal::X_VEC3D};
  double dist = Geometry::distance(ray, linal::vec3d{2, 0, 0});
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST(Distance_Ray, NextToLine_UnitDist)
{
  Ray3d ray{linal::vec3d{1, 0, 0}, linal::X_VEC3D};
  double dist = Geometry::distance(ray, linal::vec3d{2, 1, 0});
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

class Test_Ray_Creator : public ::testing::Test {
protected:
  std::vector<Ray3d> m_ray3ds;

  void SetUp() override
  {
    m_ray3ds.push_back(Ray(linal::vec3d{-1, 0, 0}, linal::X_VEC3D));
    m_ray3ds.push_back(Ray(linal::vec3d{}, linal::Y_VEC3D));
    m_ray3ds.push_back(Ray(linal::vec3d{}, linal::ZERO_VEC3D));
  }
};

TEST_F(Test_Ray_Creator, RayTestFixtureTest)
{
  std::vector<Ray3d> ray3ds;
  for (const auto& ray: m_ray3ds)
    ray3ds.push_back(ray);
  EXPECT_EQ(ray3ds.size(), 3);
}

class RayDistanceTestFixture : public ::testing::Test {
protected:
  Ray3d m_ray3d{linal::vec3d{}, linal::X_VEC3D};
};

TEST_F(RayDistanceTestFixture, PointOnRaySide)
{
  linal::vec3d vec{1, 2, 0};
  double dist = Geometry::distance(m_ray3d, vec);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(RayDistanceTestFixture, PointOrthogonalAtRayOrigin)
{
  linal::vec3d vec{0, 3, 0};
  double dist = Geometry::distance(m_ray3d, vec);
  EXPECT_DOUBLE_EQ(dist, 3);
}

TEST_F(RayDistanceTestFixture, PointBeforeRayOrigin)
{
  linal::vec3d vec{-1, 3, 0};
  double dist = Geometry::distance(m_ray3d, vec);
  EXPECT_DOUBLE_EQ(dist, linal::norm2(vec));
}
