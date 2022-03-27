#include <Geometry/GeometryAssert.hpp>
#include <Geometry/Sphere.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(Sphere, Constructor)
{
    constexpr double_t RADIUS = 1;
    Sphere sphere{ZERO_VEC3D, RADIUS};
    EXPECT_EQ(sphere.getRadius(), RADIUS);
    EXPECT_EQ(sphere.getOrigin(), ZERO_VEC3D);
}

class Sphere_Contains_Fixture : public ::testing::Test {
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

TEST(Sphere, valid)
{
    Sphere<double_t> sphere{LinAl::Vec3d{0, 0, 0}, 0.0};
#ifdef NDEBUG
    EXPECT_DEATH(GEOMETRY_PRECONDITION_SPHERE_ASSERT(sphere), "");
#else
    EXPECT_DEATH(GEOMETRY_PRECONDITION_SPHERE_DEBUG_ASSERT(sphere), "");
#endif
}
