#include <Geometry/Distance/DistancePlane.hpp>
#include <Geometry/Plane.hpp>
#include <linal/vec.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class PlaneDistanceTest : public ::testing::Test {
  protected:
    Plane<double> m_plane{linal::double3{}, linal::double3X};
    linal::double3 point{2, 0, 0};
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
    point = linal::double3{-2, 0, 0};
    double dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(PlaneDistanceTest, PointDistanceC)
{
    point = linal::double3{9, 4, -2};
    double dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 9);
}

TEST_F(PlaneDistanceTest, PointInPlane)
{
    point = linal::double3{0, 4, 0};
    double dist = distance(m_plane, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}
