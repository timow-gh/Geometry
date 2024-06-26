#include <Geometry/Sphere.hpp>
#include <Geometry/Utils/GeometryAssert.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(Sphere, Constructor)
{
  constexpr double RADIUS = 1;
  Sphere sphere{linal::double3{}, RADIUS};
  EXPECT_EQ(sphere.get_radius(), RADIUS);
  EXPECT_EQ(sphere.get_origin(), linal::double3{});
}

class Sphere_Contains_Fixture : public ::testing::Test {
protected:
  Sphere<double> m_sphere{linal::double3{}, 1.0};
};

TEST_F(Sphere_Contains_Fixture, inside)
{
  bool contains = m_sphere.contains(linal::double3{0.5, 0, 0});
  EXPECT_TRUE(contains);
}

TEST_F(Sphere_Contains_Fixture, boundary)
{
  bool contains = m_sphere.contains(linal::double3{1.0, 0, 0});
  EXPECT_TRUE(contains);
}

TEST_F(Sphere_Contains_Fixture, outside)
{
  bool contains = m_sphere.contains(linal::double3{1.5, 0, 0});
  EXPECT_FALSE(contains);
}
