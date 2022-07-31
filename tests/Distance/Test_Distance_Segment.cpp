

#include <Geometry/Distance/DistanceSegment.hpp>
#include <Geometry/Segment.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class Segment_VecDistance_Test
    : public ::testing::Test
{
  protected:
    Segment3d m_segment3d{Vec3d{}, LinAl::X_VEC3D};
    LinAl::Vec3d m_point{0};
};

TEST_F(Segment_VecDistance_Test, PointAtSegSource)
{
    double_t dist = distance(m_segment3d, m_point);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointAtSegSource_ArgOrder)
{
    double_t dist = distance(m_point, m_segment3d);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointAtSegTarget)
{
    LinAl::Vec3d point = LinAl::Vec3d{1, 0, 0};
    double_t dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointUnitDistAtSegSource)
{
    LinAl::Vec3d point{0, 1, 0};
    double_t dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 1);
}

TEST_F(Segment_VecDistance_Test, PointUnitDistAtSegTarget)
{
    LinAl::Vec3d point{1, 1, 0};
    double_t dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 1);
}

TEST_F(Segment_VecDistance_Test, PointAtSegMidPoint)
{
    LinAl::Vec3d point{0.5, 0, 0};
    double_t dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointCollinearBeforeSegment)
{
    LinAl::Vec3d point{-3, 0, 0};
    double_t dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 3);
}

TEST_F(Segment_VecDistance_Test, PointCollinearAfterSegment)
{
    LinAl::Vec3d point{3, 0, 0};
    double_t dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(Segment_VecDistance_Test, PointOffsetAtSegmentMidPoint)
{
    LinAl::Vec3d point{0.5, 3, 0};
    double_t dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 3);

    point = LinAl::Vec3d{0.5, -3, 0};
    dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 3);
}

