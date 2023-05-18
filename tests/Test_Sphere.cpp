#include <Geometry/GeometryAssert.hpp>
#include <Geometry/Sphere.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(Sphere, Constructor)
{
    constexpr double RADIUS = 1;
    Sphere sphere{linal::ZERO_VEC3D, RADIUS};
    EXPECT_EQ(sphere.getRadius(), RADIUS);
    EXPECT_EQ(sphere.getOrigin(), linal::ZERO_VEC3D);
}

class Sphere_Contains_Fixture : public ::testing::Test {
  protected:
    Sphere<double> m_sphere{linal::ZERO_VEC3D, 1.0};
};

TEST_F(Sphere_Contains_Fixture, inside)
{
    bool contains = m_sphere.contains(linal::Vec3d{0.5, 0, 0});
    EXPECT_TRUE(contains);
}

TEST_F(Sphere_Contains_Fixture, boundary)
{
    bool contains = m_sphere.contains(linal::Vec3d{1.0, 0, 0});
    EXPECT_TRUE(contains);
}

TEST_F(Sphere_Contains_Fixture, outside)
{
    bool contains = m_sphere.contains(linal::Vec3d{1.5, 0, 0});
    EXPECT_FALSE(contains);
}
