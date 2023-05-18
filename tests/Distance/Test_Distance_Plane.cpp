#include <Geometry/Distance/DistancePlane.hpp>
#include <Geometry/Plane.hpp>
#include <linal/Vec.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class PlaneDistanceTest : public ::testing::Test {
  protected:
    Plane<double> m_plane{linal::Vec3d{}, linal::X_VEC3D};
    linal::Vec3d point{2, 0, 0};
};

TEST_F(PlaneDistanceTest, PointDistanceA)
{
    double dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(PlaneDistanceTest, PointDistanceA_ArgOrder)
{
    double dist = distance(point, m_plane);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(PlaneDistanceTest, PointDistanceB)
{
    point = linal::Vec3d{-2, 0, 0};
    double dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(PlaneDistanceTest, PointDistanceC)
{
    point = linal::Vec3d{9, 4, -2};
    double dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 9);
}

TEST_F(PlaneDistanceTest, PointInPlane)
{
    point = linal::Vec3d{0, 4, 0};
    double dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}
