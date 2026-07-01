

#include <Geometry/Distance/DistanceSegment.hpp>
#include <Geometry/Segment.hpp>
#include <linal/vec.hpp>
#include <gtest/gtest.h>
#include <cmath>

using namespace Geometry;

class Segment_VecDistance_Test : public ::testing::Test {
  protected:
    Segment3d m_segment3d{linal::double3{}, linal::double3X};
    linal::double3 m_point{0};
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
    linal::double3 point = linal::double3{1, 0, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointUnitDistAtSegSource)
{
    linal::double3 point{0, 1, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 1);
}

TEST_F(Segment_VecDistance_Test, PointUnitDistAtSegTarget)
{
    linal::double3 point{1, 1, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 1);
}

TEST_F(Segment_VecDistance_Test, PointAtSegMidPoint)
{
    linal::double3 point{0.5, 0, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, PointCollinearBeforeSegment)
{
    linal::double3 point{-3, 0, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 3);
}

TEST_F(Segment_VecDistance_Test, PointCollinearAfterSegment)
{
    linal::double3 point{3, 0, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(Segment_VecDistance_Test, PointOffsetAtSegmentMidPoint)
{
    linal::double3 point{0.5, 3, 0};
    double dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 3);

    point = linal::double3{0.5, -3, 0};
    dist = distance(m_segment3d, point);
    EXPECT_DOUBLE_EQ(dist, 3);
}

TEST(Segment_SegmentDistance_Test, SkewSegments)
{
    Segment3d a{linal::double3{0, 0, 0}, linal::double3{1, 0, 0}};
    Segment3d b{linal::double3{0, 1, 1}, linal::double3{1, 1, 1}};
    double dist = distance(a, b);
    EXPECT_DOUBLE_EQ(dist, std::sqrt(2.0));
}

TEST(Segment_SegmentDistance_Test, ParallelNonCollinear_DirectlyAcross)
{
    Segment3d a{linal::double3{0, 0, 0}, linal::double3{1, 0, 0}};
    Segment3d b{linal::double3{0, 2, 0}, linal::double3{1, 2, 0}};
    double dist = distance(a, b);
    EXPECT_DOUBLE_EQ(dist, 2.0);
    EXPECT_GE(dist, 0.0);
}

TEST(Segment_SegmentDistance_Test, ParallelNonCollinear_Offset)
{
    Segment3d a{linal::double3{0, 0, 0}, linal::double3{1, 0, 0}};
    Segment3d b{linal::double3{2, 2, 0}, linal::double3{3, 2, 0}};
    double dist = distance(a, b);
    // Nearest points are a's target (1,0,0) and b's source (2,2,0).
    double expected = linal::length(linal::double3{1, 0, 0} - linal::double3{2, 2, 0});
    EXPECT_DOUBLE_EQ(dist, expected);
    EXPECT_GE(dist, 0.0);
}

TEST(Segment_SegmentDistance_Test, Collinear_NonOverlapping)
{
    Segment3d a{linal::double3{0, 0, 0}, linal::double3{1, 0, 0}};
    Segment3d b{linal::double3{3, 0, 0}, linal::double3{4, 0, 0}};
    double dist = distance(a, b);
    EXPECT_DOUBLE_EQ(dist, 2.0);
    EXPECT_GE(dist, 0.0);
}

TEST(Segment_SegmentDistance_Test, Collinear_Overlapping)
{
    Segment3d a{linal::double3{0, 0, 0}, linal::double3{2, 0, 0}};
    Segment3d b{linal::double3{1, 0, 0}, linal::double3{3, 0, 0}};
    double dist = distance(a, b);
    EXPECT_DOUBLE_EQ(dist, 0.0);
    EXPECT_GE(dist, 0.0);
}
TEST(Segment_VecDistance_ZeroLengthTest, ZeroLengthSegment_ReturnsPointToPointDistance)
{
    linal::double3 p{1, 1, 1};
    Segment3d degenerateSegment{p, p};
    linal::double3 point{4, 5, 1};

    double dist = distance(degenerateSegment, point);
    double expected = linal::length(point - p);

    EXPECT_DOUBLE_EQ(dist, expected);
    EXPECT_FALSE(std::isnan(dist));
}

// Regression tests for a bug where `parameter` was normalized by
// `linal::length(dir)` instead of `linal::dot(dir, dir)`. This is invisible
// for unit-length segments (like m_segment3d above), so these use a
// non-unit-length (length 2) segment to expose it.
TEST_F(Segment_VecDistance_Test, NonUnitLength_PointAtSegMidPoint)
{
    Segment3d segment{linal::double3{0, 0, 0}, linal::double3{2, 0, 0}};
    linal::double3 point{1, 0, 0};
    double dist = distance(segment, point);
    EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistance_Test, NonUnitLength_PointOffsetAtSegmentMidPoint)
{
    Segment3d segment{linal::double3{0, 0, 0}, linal::double3{2, 0, 0}};
    linal::double3 point{1, 3, 0};
    double dist = distance(segment, point);
    EXPECT_DOUBLE_EQ(dist, 3);
}

TEST_F(Segment_VecDistance_Test, NonUnitLength_PointBeyondTarget)
{
    Segment3d segment{linal::double3{0, 0, 0}, linal::double3{2, 0, 0}};
    linal::double3 point{3, 0, 0};
    double dist = distance(segment, point);
    EXPECT_DOUBLE_EQ(dist, 1);
}

struct SegmentVecDistanceCase
{
  double segmentLength;      // length of the segment, along +x from the origin
  linal::double3 point;      // query point
  double expectedDistance;
};

class SegmentVecDistance_LengthSweep : public ::testing::TestWithParam<SegmentVecDistanceCase>
{
};

TEST_P(SegmentVecDistance_LengthSweep, MatchesExpectedDistance)
{
  const auto& c = GetParam();
  Segment3d segment{linal::double3{0, 0, 0}, linal::double3{c.segmentLength, 0, 0}};
  EXPECT_DOUBLE_EQ(distance(segment, c.point), c.expectedDistance);
}

INSTANTIATE_TEST_SUITE_P(
  VariousLengths,
  SegmentVecDistance_LengthSweep,
  ::testing::Values(
    // length 1 (unit, matches the old fixture — must still pass, proves no regression)
    SegmentVecDistanceCase{1.0, linal::double3{0.5, 0, 0}, 0.0},
    // length 2: midpoint on segment must be 0 -- this is the case that was returning 1 before the fix
    SegmentVecDistanceCase{2.0, linal::double3{1.0, 0, 0}, 0.0},
    // length 2: perpendicular offset from a point 3/4 along the segment
    SegmentVecDistanceCase{2.0, linal::double3{1.5, 4.0, 0}, 4.0},
    // length 0.5: point beyond target, clamped
    SegmentVecDistanceCase{0.5, linal::double3{2.0, 0, 0}, 1.5},
    // length 10: point before source, clamped
    SegmentVecDistanceCase{10.0, linal::double3{-3.0, 4.0, 0}, 5.0}
  )
);