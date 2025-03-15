#include <Geometry/Intersect/IntersectSegment.hpp>
#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

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

TEST(Segment3d_Ray3d_Intersection, Intersection_Orthogonal)
{
  Segment3d seg{linal::double3{1, 1, 0}, linal::double3{1, 1, 0} + linal::double3{1, 0, 0}};
  Ray3d ray{linal::double3{1, 1, 0}, linal::double3{0, 0, 1}};
  auto res = Geometry::intersect(seg, ray);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{1, 1, 0}), *res);
}

TEST(Segment3d_Ray3d_Intersection, Intersection_45deg)
{
  Segment3d seg{linal::double3{0, 1, 1}, linal::double3{5, 1, 1}};
  Ray3d ray{linal::double3{0, 0, 0}, linal::double3{1, 1, 1}};
  auto res = Geometry::intersect(seg, ray);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{1, 1, 1}), *res);
}

TEST(Segment3d_Ray3d_Intersection, Skew)
{
  Segment3d seg{linal::double3{0, 0, 0}, linal::double3{0, 0, 0} + linal::double3X};
  Ray3d ray{linal::double3{1, 1, 1}, linal::double3{1, 1, 1} + linal::double3Y};
  auto res = Geometry::intersect(seg, ray);
  EXPECT_FALSE(res);
}

TEST(Segment3d_Ray3d_Intersection, Collinear)
{
  Segment3d seg{linal::double3{0, 0, 0}, linal::double3X};
  Ray3d ray{linal::double3{-2, 0, 0}, linal::double3X * 2};
  auto res = Geometry::intersect(seg, ray);
  EXPECT_FALSE(res);
}

TEST(Segment3d_Ray3d_Intersection, Parallel)
{
  Segment3d seg{linal::double3{0, 0, 2}, linal::double3{0, 0, 2} + linal::double3X};
  Ray3d ray{linal::double3{0, 1, 3}, linal::double3X};
  auto res = Geometry::intersect(seg, ray);
  EXPECT_FALSE(res);
}

TEST(Segment3d_Ray3d_Intersection, Intersection_Source)
{
  linal::double3 source{2, 1, 0};
  Segment3d seg{source, linal::double3{0, 0, 0}};
  Ray3d ray{source, linal::double3{3, 4, 1}};
  auto res = Geometry::intersect(seg, ray);
  EXPECT_TRUE(res);
  EXPECT_EQ(source, *res);
}

TEST(Segment3d_Ray3d_Intersection, Intersection_Target)
{
  linal::double3 target{2, 1, 0};
  Segment3d seg{linal::double3{0, 0, 0}, target};
  Ray3d ray{target, linal::double3X};
  auto res = Geometry::intersect(seg, ray);
  EXPECT_TRUE(res);
  EXPECT_EQ(target, *res);
}

TEST(Segment3d_Ray3d_Intersection, AtPoint22)
{
  Segment3d seg{linal::double3{0, 2, 1}, linal::double3{4, 2, 1}};
  Ray3d ray{linal::double3{2, 0, 1}, linal::double3{0, 4, 0}};
  auto res = Geometry::intersect(seg, ray);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{2, 2, 1}), *res);
}

TEST(Segment3d_Ray3d_Intersection, RayPointsAtSegment)
{
  linal::double3 segTarget{6, 0, 0};
  Segment3d seg{linal::double3{0, 0, 0}, segTarget};

  linal::double3 rayOrigin = linal::double3{15, 15, 15};
  linal::double3 rayTarget = linal::double3{0, 0, 0};

  // Segment origin
  linal::double3 rayDir = rayTarget - rayOrigin;
  Ray3d ray{linal::double3{15, 15, 15}, rayDir};
  auto res = Geometry::intersect(seg, ray);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{0, 0, 0}), *res);

  // Segment mid point
  rayTarget = linal::double3{3, 0, 0};
  rayDir = rayTarget - rayOrigin;
  ray = Ray3d{rayOrigin, rayDir};
  res = Geometry::intersect(seg, ray);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{3, 0, 0}), *res);

  // Segment target
  rayTarget = segTarget;
  rayDir = rayTarget - rayOrigin;
  ray = Ray3d{rayOrigin, rayDir};
  res = Geometry::intersect(seg, ray);
  EXPECT_TRUE(res);
  EXPECT_EQ(segTarget, *res);
}
