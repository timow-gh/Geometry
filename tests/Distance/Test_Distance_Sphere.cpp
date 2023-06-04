

#include <Geometry/Distance/DistanceSphere.hpp>
#include <Geometry/Sphere.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class Vec_Sphere_Distance_Test : public ::testing::Test {
  protected:
    Sphere<double> m_sphere{linal::ZERO_VEC3D, 3.0};
};

TEST_F(Vec_Sphere_Distance_Test, InsideSphere)
{
    linal::vec3d vec = {2, 0, 0};
    double dist = distance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Vec_Sphere_Distance_Test, SphereBoundary)
{
    linal::vec3d vec = {3, 0, 0};
    double dist = distance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Vec_Sphere_Distance_Test, OutsideOfSphere)
{
    linal::vec3d vec = {4, 0, 0};
    double dist = distance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Vec_Sphere_Distance_Test, Signed_InsideSphere)
{
    linal::vec3d vec = {2, 0, 0};
    double dist = signedDistance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, -1.0);
}

TEST_F(Vec_Sphere_Distance_Test, Signed_SphereBoundary)
{
    linal::vec3d vec = {3, 0, 0};
    double dist = signedDistance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Vec_Sphere_Distance_Test, Signed_OutsideOfSphere)
{
    linal::vec3d vec = {4, 0, 0};
    double dist = signedDistance(vec, m_sphere);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}
