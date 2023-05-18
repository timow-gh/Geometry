

#include <Geometry/Distance/DistanceSphere.hpp>
#include <Geometry/Sphere.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace linal;

class Vec_Sphere_Distance_Test : public ::testing::Test {
  protected:
    Sphere<double> m_sphere{ZERO_VEC3D, 3.0};
};

TEST_F(Vec_Sphere_Distance_Test, InsideSphere)
{
    linal::Vec3d vec = {2, 0, 0};
    double dist = distance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Vec_Sphere_Distance_Test, SphereBoundary)
{
    linal::Vec3d vec = {3, 0, 0};
    double dist = distance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Vec_Sphere_Distance_Test, OutsideOfSphere)
{
    linal::Vec3d vec = {4, 0, 0};
    double dist = distance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Vec_Sphere_Distance_Test, Signed_InsideSphere)
{
    linal::Vec3d vec = {2, 0, 0};
    double dist = signedDistance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, -1.0);
}

TEST_F(Vec_Sphere_Distance_Test, Signed_SphereBoundary)
{
    linal::Vec3d vec = {3, 0, 0};
    double dist = signedDistance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Vec_Sphere_Distance_Test, Signed_OutsideOfSphere)
{
    linal::Vec3d vec = {4, 0, 0};
    double dist = signedDistance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}
