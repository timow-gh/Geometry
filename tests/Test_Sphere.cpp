#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Geometry/Sphere.hpp"
#include "gtest/gtest.h"

using namespace Geometry;
using namespace LinAl;

TEST(Sphere, Constructor)
{
    constexpr double_t RADIUS = 1;
    Sphere sphere{ZERO_VEC3D, RADIUS};
    EXPECT_EQ(sphere.getRadius(), RADIUS);
    EXPECT_EQ(sphere.getOrigin(), ZERO_VEC3D);
}

TEST(Sphere, Equality)
{
    constexpr double_t RADIUS = 1;
    Sphere sphereA{ZERO_VEC3D, RADIUS};
    Sphere sphereB{ZERO_VEC3D, RADIUS};
    EXPECT_EQ(sphereA, sphereB);
}

TEST(Sphere, Inequality)
{
    constexpr double_t RADIUS_A = 1;
    Sphere sphereA{ZERO_VEC3D, RADIUS_A};
    constexpr double_t RADIUS_B = 2;
    Sphere sphereB{ZERO_VEC3D, RADIUS_B};
    EXPECT_NE(sphereA, sphereB);
}

class Sphere_Contains_Fixture
    : public ::testing::Test
{
  protected:
    Sphere<double_t> m_sphere{LinAl::ZERO_VEC3D, 1.0};
};

TEST_F(Sphere_Contains_Fixture, inside)
{
    bool contains = m_sphere.contains(LinAl::Vec3d{0.5, 0, 0});
    EXPECT_TRUE(contains);
}

TEST_F(Sphere_Contains_Fixture, boundary)
{
    bool contains = m_sphere.contains(LinAl::Vec3d{1.0, 0, 0});
    EXPECT_TRUE(contains);
}

TEST_F(Sphere_Contains_Fixture, outside)
{
    bool contains = m_sphere.contains(LinAl::Vec3d{1.5, 0, 0});
    EXPECT_FALSE(contains);
}

#pragma clang diagnostic pop