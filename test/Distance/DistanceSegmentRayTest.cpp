#include <Geometry/Distance/DistanceSegment.hpp>
#include <Geometry/Segment.hpp>
#include <cmath>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

class Distance_Segment_Ray_Test : public ::testing::Test {
protected:
  Segment3d m_segment{linal::double3{}, linal::double3X};
};

TEST_F(Distance_Segment_Ray_Test, parallel_before_seg_source_away_from_seg)
{
  Ray3d ray{linal::double3{-1, 0, 1}, -linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 1.4142135623730951);
}

TEST_F(Distance_Segment_Ray_Test, parallel_before_seg_source_toward_seg)
{
  Ray3d ray{linal::double3{-1, 0, 1}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Distance_Segment_Ray_Test, parallel_inside_seg)
{
  Ray3d ray{linal::double3{0.5, 0, 1}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Distance_Segment_Ray_Test, parallel_after_seg_source_toward_seg)
{
  Ray3d ray{linal::double3{2, 0, 1}, -linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Distance_Segment_Ray_Test, parallel_after_seg_source_away_from_seg)
{
  Ray3d ray{linal::double3{2, 0, 1}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 1.4142135623730951);
}

TEST_F(Distance_Segment_Ray_Test, collinear_before_seg_source_away_from_seg)
{
  Ray3d ray{linal::double3{-1, 0, 0}, -linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Distance_Segment_Ray_Test, collinear_before_seg_source_toward_seg)
{
  Ray3d ray{linal::double3{-1, 0, 0}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Segment_Ray_Test, collinear_seg_source_away_from_seg)
{
  Ray3d ray{linal::double3{0, 0, 0}, -linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Segment_Ray_Test, collinear_inside_seg)
{
  Ray3d ray{linal::double3{0.5, 0, 0}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Segment_Ray_Test, collinear_seg_target_away_from_seg)
{
  Ray3d ray{linal::double3{1, 0, 0}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Segment_Ray_Test, collinear_after_seg_target_away_from_seg)
{
  Ray3d ray{linal::double3{1.5, 0, 0}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0.5);
}

TEST_F(Distance_Segment_Ray_Test, collinear_after_seg_target_toward_seg)
{
  Ray3d ray{linal::double3{1.5, 0, 0}, -linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Segment_Ray_Test, perpendicular_before_source)
{
  Ray3d ray{linal::double3{-2, 0, 0}, linal::double3Y};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 2);
  dist = distance(ray, m_segment);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(Distance_Segment_Ray_Test, perpendicular_source)
{
  Ray3d ray{linal::double3{0, 0, 0}, linal::double3Y};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Distance_Segment_Ray_Test, perpendicular_inside_segment)
{
  Ray3d ray{linal::double3{0.6, 0, 0}, linal::double3Y};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Distance_Segment_Ray_Test, perpendicular_target)
{
  Ray3d ray{linal::double3{1, 0, 0}, linal::double3Y};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Distance_Segment_Ray_Test, perpendicular_after_target)
{
  Ray3d ray{linal::double3{5, 0, 0}, linal::double3Y};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 4);
}

TEST_F(Distance_Segment_Ray_Test, perpendicular_ray_source)
{
  Ray3d ray{linal::double3{0.5, 1.0, 0}, linal::double3Y};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(Distance_Segment_Ray_Test, perpendicular_ray)
{
  Ray3d ray{linal::double3{0.5, -1.0, 0}, linal::double3Y};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Segment_Ray_Test, skew_ray)
{
  Ray3d ray{linal::double3{0.5, -1.0, 2.0}, linal::double3Y};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 2.0);
}

// Regression tests for a bug where `parameter` (used only for branch
// classification in the parallel/collinear case) was normalized by
// `linal::length(dir)` instead of `linal::dot(dir, dir)`. This is invisible
// for unit-length segments (like m_segment above), so these use a
// non-unit-length (length 2) segment to expose it.
class Distance_Segment_Ray_NonUnitLength_Test : public ::testing::Test {
protected:
  Segment3d m_segment{linal::double3{0, 0, 0}, linal::double3{2, 0, 0}};
};

TEST_F(Distance_Segment_Ray_NonUnitLength_Test, parallel_offset_correct_parameter_inside_range)
{
  // diffVec = (1.5, 3, 0), dir = (2, 0, 0).
  // Correct parameter = dot(diffVec,dir)/dot(dir,dir) = 3/4 = 0.75 -> in [0,1] -> lineDist.
  // Buggy parameter = dot(diffVec,dir)/length(dir) = 3/2 = 1.5 -> > 1 -> wrong branch,
  // would incorrectly return distance to segment target (~3.041) instead of 3.
  Ray3d ray{linal::double3{1.5, 3, 0}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 3.0);
}

TEST_F(Distance_Segment_Ray_NonUnitLength_Test, collinear_inside_seg_nonunit)
{
  Ray3d ray{linal::double3{1.0, 0, 0}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Segment_Ray_NonUnitLength_Test, collinear_after_seg_target_toward_seg_nonunit)
{
  Ray3d ray{linal::double3{3.0, 0, 0}, -linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

// Regression tests for a zero-length segment (source == target) reaching
// distance(Segment, Ray). Previously this crashed via Line's constructor
// assert (Debug builds) or fell through to a disengaged std::optional
// dereference / division by zero (Release builds). The fix guards on
// dirLength and delegates to the existing distance(vec, Ray) overload.
class Distance_Segment_Ray_ZeroLengthTest : public ::testing::Test {
protected:
  Segment3d m_segment{linal::double3{1, 0, 0}, linal::double3{1, 0, 0}};
};

TEST_F(Distance_Segment_Ray_ZeroLengthTest, ray_pointing_toward_point)
{
  // Ray origin at (1, -2, 0) pointing in +Y reaches the segment's point (1,0,0)
  // at ray-parameter 2 >= 0, so distance should be the straight-line distance, 0.
  Ray3d ray{linal::double3{1, -2, 0}, linal::double3Y};
  double dist = distance(m_segment, ray);
  EXPECT_FALSE(std::isnan(dist));
  EXPECT_GE(dist, 0.0);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Segment_Ray_ZeroLengthTest, ray_closest_point_ahead_of_origin)
{
  // Ray origin at (0, 0, 0) pointing in +X; closest approach to (1,0,0) is at
  // ray-parameter 1 >= 0, so distance is the point-to-point distance, 0.
  Ray3d ray{linal::double3{0, 0, 0}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_FALSE(std::isnan(dist));
  EXPECT_GE(dist, 0.0);
  EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Segment_Ray_ZeroLengthTest, ray_pointing_away_clamped_to_origin)
{
  // Ray origin at (3, 0, 0) pointing in +X, away from the segment's point
  // (1,0,0). The closest point on the ray is clamped to its origin, so the
  // expected distance is from (1,0,0) to (3,0,0), i.e. 2 -- not to any point
  // "behind" the ray's start.
  Ray3d ray{linal::double3{3, 0, 0}, linal::double3X};
  double dist = distance(m_segment, ray);
  EXPECT_FALSE(std::isnan(dist));
  EXPECT_GE(dist, 0.0);
  EXPECT_DOUBLE_EQ(dist, 2.0);
}

TEST_F(Distance_Segment_Ray_ZeroLengthTest, no_crash_via_ray_segment_overload)
{
  Ray3d ray{linal::double3{3, 0, 0}, linal::double3X};
  double dist = distance(ray, m_segment);
  EXPECT_FALSE(std::isnan(dist));
  EXPECT_GE(dist, 0.0);
  EXPECT_DOUBLE_EQ(dist, 2.0);
}