

#include <Geometry/Distance/DistanceSegment.hpp>
#include <Geometry/Segment.hpp>
#include <linal/Vec3.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

class Segment_VecDistance_Test : public ::testing::Test {
  protected:
    Segment3d m_segment3d{linal::Vec3d{}, linal::X_VEC3D};
    linal::Vec3d m_point{0};
};

TEST_F(Segment_VecDistance_Test, PointAtSegSource)
{
    double dist = distance(m_segment3d, m_point);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointAtSegSource_ArgOrder)
{
    double dist = distance(m_point, m_segment3d);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointAtSegTarget)
{
    linal::Vec3d point = linal::Vec3d{1, 0, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointUnitDistAtSegSource)
{
    linal::Vec3d point{0, 1, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 1);
}

TEST_F(Segment_VecDistance_Test, PointUnitDistAtSegTarget)
{
    linal::Vec3d point{1, 1, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 1);
}

TEST_F(Segment_VecDistance_Test, PointAtSegMidPoint)
{
    linal::Vec3d point{0.5, 0, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointCollinearBeforeSegment)
{
    linal::Vec3d point{-3, 0, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 3);
}

TEST_F(Segment_VecDistance_Test, PointCollinearAfterSegment)
{
    linal::Vec3d point{3, 0, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(Segment_VecDistance_Test, PointOffsetAtSegmentMidPoint)
{
    linal::Vec3d point{0.5, 3, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 3);

    point = linal::Vec3d{0.5, -3, 0};
    dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 3);
}
