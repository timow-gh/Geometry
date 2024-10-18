#include <Geometry/Distance/DistanceSegment.hpp>
#include <Geometry/Segment.hpp>
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