#include <Geometry/Intersect/IntersectSegment.hpp>
#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(Segment3d_Intersection, Intersection_Orthogonal)
{
  Segment3d segA{linal::vec3d{1, 1, 0}, linal::vec3d{1, 1, 0} + linal::vec3d{1, 0, 0}};
  Segment3d segB{linal::vec3d{1, 1, 0}, linal::vec3d{1, 1, 0} + linal::vec3d{0, 1, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::vec3d{1, 1, 0}), *res);
}

TEST(Segment3d_Intersection, Intersection_45deg)
{
  Segment3d segA{linal::vec3d{0, 1, 1}, linal::vec3d{5, 1, 1}};
  Segment3d segB{linal::vec3d{0, 0, 0}, linal::vec3d{3, 3, 3}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::vec3d{1, 1, 1}), *res);
}

TEST(Segment3d_Intersection, Intersection_neg45deg)
{
  Segment3d segA{linal::vec3d{0, 1, 1}, linal::vec3d{2, 1, 1}};
  Segment3d segB{linal::vec3d{0, 2, 1}, linal::vec3d{2, 0, 1}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::vec3d{1, 1, 1}), *res);
}

TEST(Segment3d_Intersection, Skew)
{
  Segment3d segA{linal::vec3d{0, 0, 0}, linal::vec3d{0, 0, 0} + linal::X_VEC3D};
  Segment3d segB{linal::vec3d{1, 1, 1}, linal::vec3d{1, 1, 1} + linal::Y_VEC3D};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment3d_Intersection, Collinear)
{
  Segment3d segA{linal::vec3d{0, 0, 0}, linal::vec3d{1, 0, 0}};
  Segment3d segB{linal::vec3d{-2, 0, 0}, linal::vec3d{-1, 0, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment3d_Intersection, Parallel)
{
  Segment3d segA{linal::vec3d{0, 0, 2}, linal::vec3d{0, 0, 2} + linal::vec3d{1, 0, 0}};
  Segment3d segB{linal::vec3d{0, 1, 3}, linal::vec3d{0, 1, 3} + linal::vec3d{1, 0, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment2d_Intersection, Orthogonal)
{
  Segment2d segA{linal::vec2d{1, 1}, linal::vec2d{1, 1} + linal::vec2d{3, 0}};
  Segment2d segB{linal::vec2d{1, 1}, linal::vec2d{1, 1} + linal::vec2d{0, 2}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::vec2d{1, 1}), *res);
}

TEST(Segment2d_Intersection, Orthogonal_No_Intersection)
{
  Segment2d segA{linal::vec2d{0, 0}, linal::vec2d{0, 0} + linal::X_VEC2D};
  Segment2d segB{linal::vec2d{1, 1}, linal::vec2d{1, 1} + linal::Y_VEC2D};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment2d_Intersection, Intersection_45deg)
{
  Segment2d segA{linal::vec2d{0, 1}, linal::vec2d{2, 1}};
  Segment2d segB{linal::vec2d{0, 0}, linal::vec2d{3, 3}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::vec2d{1, 1}), *res);
}

TEST(Segment2d_Intersection, Intersection_neg45deg)
{
  Segment2d segA{linal::vec2d{0, 1}, linal::vec2d{2, 1}};
  Segment2d segB{linal::vec2d{0, 2}, linal::vec2d{2, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::vec2d{1, 1}), *res);
}

TEST(Segment2d_Intersection, Intersection_SourceTarget)
{
  linal::vec2d source{2, 1};
  Segment2d segA{source, linal::vec2d{0, 0}};
  Segment2d segB{source, linal::vec2d{3, 4}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ(source, *res);
}

TEST(Segment2d_Intersection, Intersection_TargetSource)
{
  linal::vec2d target{2, 1};
  Segment2d segA{linal::vec2d{0, 1}, target};
  Segment2d segB{linal::vec2d{1, -1}, target};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ(target, *res);
}

TEST(Segment2d_Intersection, Collinear)
{
  Segment2d segA{linal::vec2d{0, 0}, linal::vec2d{1, 0}};
  Segment2d segB{linal::vec2d{-2, 0}, linal::vec2d{-1, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment2d_Intersection, Parallel)
{
  Segment2d segA{linal::vec2d{0, 0}, linal::vec2d{0, 0} + linal::vec2d{1, 0}};
  Segment2d segB{linal::vec2d{0, 1}, linal::vec2d{0, 1} + linal::vec2d{1, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}
