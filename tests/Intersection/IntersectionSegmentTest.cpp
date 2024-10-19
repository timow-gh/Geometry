#include <Geometry/Intersect/IntersectSegment.hpp>
#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(Segment3d_Intersection, Intersection_Orthogonal)
{
  Segment3d segA{linal::double3{1, 1, 0}, linal::double3{1, 1, 0} + linal::double3{1, 0, 0}};
  Segment3d segB{linal::double3{1, 1, 0}, linal::double3{1, 1, 0} + linal::double3{0, 1, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{1, 1, 0}), *res);
}

TEST(Segment3d_Intersection, Intersection_45deg)
{
  Segment3d segA{linal::double3{0, 1, 1}, linal::double3{5, 1, 1}};
  Segment3d segB{linal::double3{0, 0, 0}, linal::double3{3, 3, 3}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{1, 1, 1}), *res);
}

TEST(Segment3d_Intersection, Intersection_neg45deg)
{
  Segment3d segA{linal::double3{0, 1, 1}, linal::double3{2, 1, 1}};
  Segment3d segB{linal::double3{0, 2, 1}, linal::double3{2, 0, 1}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{1, 1, 1}), *res);
}

TEST(Segment3d_Intersection, Skew)
{
  Segment3d segA{linal::double3{0, 0, 0}, linal::double3{0, 0, 0} + linal::double3X};
  Segment3d segB{linal::double3{1, 1, 1}, linal::double3{1, 1, 1} + linal::double3Y};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment3d_Intersection, Collinear)
{
  Segment3d segA{linal::double3{0, 0, 0}, linal::double3{1, 0, 0}};
  Segment3d segB{linal::double3{-2, 0, 0}, linal::double3{-1, 0, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment3d_Intersection, Parallel)
{
  Segment3d segA{linal::double3{0, 0, 2}, linal::double3{0, 0, 2} + linal::double3{1, 0, 0}};
  Segment3d segB{linal::double3{0, 1, 3}, linal::double3{0, 1, 3} + linal::double3{1, 0, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment2d_Intersection, Orthogonal)
{
  Segment2d segA{linal::double2{1, 1}, linal::double2{1, 1} + linal::double2{3, 0}};
  Segment2d segB{linal::double2{1, 1}, linal::double2{1, 1} + linal::double2{0, 2}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double2{1, 1}), *res);
}

TEST(Segment2d_Intersection, Orthogonal_No_Intersection)
{
  Segment2d segA{linal::double2{0, 0}, linal::double2{0, 0} + linal::double2X};
  Segment2d segB{linal::double2{1, 1}, linal::double2{1, 1} + linal::double2Y};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment2d_Intersection, Intersection_45deg)
{
  Segment2d segA{linal::double2{0, 1}, linal::double2{2, 1}};
  Segment2d segB{linal::double2{0, 0}, linal::double2{3, 3}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double2{1, 1}), *res);
}

TEST(Segment2d_Intersection, Intersection_neg45deg)
{
  Segment2d segA{linal::double2{0, 1}, linal::double2{2, 1}};
  Segment2d segB{linal::double2{0, 2}, linal::double2{2, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double2{1, 1}), *res);
}

TEST(Segment2d_Intersection, Intersection_SourceTarget)
{
  linal::double2 source{2, 1};
  Segment2d segA{source, linal::double2{0, 0}};
  Segment2d segB{source, linal::double2{3, 4}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ(source, *res);
}

TEST(Segment2d_Intersection, Intersection_TargetSource)
{
  linal::double2 target{2, 1};
  Segment2d segA{linal::double2{0, 1}, target};
  Segment2d segB{linal::double2{1, -1}, target};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_TRUE(res);
  EXPECT_EQ(target, *res);
}

TEST(Segment2d_Intersection, Collinear)
{
  Segment2d segA{linal::double2{0, 0}, linal::double2{1, 0}};
  Segment2d segB{linal::double2{-2, 0}, linal::double2{-1, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}

TEST(Segment2d_Intersection, Parallel)
{
  Segment2d segA{linal::double2{0, 0}, linal::double2{0, 0} + linal::double2{1, 0}};
  Segment2d segB{linal::double2{0, 1}, linal::double2{0, 1} + linal::double2{1, 0}};
  auto res = Geometry::intersect(segA, segB);
  EXPECT_FALSE(res);
}
