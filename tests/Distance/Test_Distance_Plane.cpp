#include <Geometry/Distance/DistancePlane.hpp>
#include <Geometry/Plane.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class PlaneDistanceTest
    : public ::testing::Test
{
  protected:
    Plane<double_t> m_plane{Vec3d{}, LinAl::X_VEC3D};
    LinAl::Vec3d point{2, 0, 0};
};

TEST_F(PlaneDistanceTest, PointDistanceA)
{
    double_t dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(PlaneDistanceTest, PointDistanceA_ArgOrder)
{
    double_t dist = distance(point, m_plane);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(PlaneDistanceTest, PointDistanceB)
{
    point = LinAl::Vec3d{-2, 0, 0};
    double_t dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(PlaneDistanceTest, PointDistanceC)
{
    point = LinAl::Vec3d{9, 4, -2};
    double_t dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 9);
}

TEST_F(PlaneDistanceTest, PointInPlane)
{
    point = LinAl::Vec3d{0, 4, 0};
    double_t dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}
