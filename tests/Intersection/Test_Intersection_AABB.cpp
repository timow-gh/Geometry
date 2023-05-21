#include <Geometry/AABB.hpp>
#include <Geometry/Intersect/IntersectAABB.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(AABB_Is_Intersecting, is_intersecting_false)
{
  AABB2f aabb1{{0, 0}, 1.0F};
  AABB2f aabb2{{1.0F, 0}, 1.0F};
  // The AABBs are not intersecting, only touching.
  EXPECT_FALSE(Geometry::is_intersecting(aabb1, aabb2));
}

TEST(AABB_Is_Intersecting, is_intersecting_true)
{
  AABB2f aabb1{{0, 0}, 1.0F};
  AABB2f aabb2{{0.5F, 0}, 1.0F};
  EXPECT_TRUE(Geometry::is_intersecting(aabb1, aabb2));
}

TEST(AABB_Is_Intersecting, is_intersecting_corner)
{
  AABB2f aabb1{{0, 0}, 0.5F};
  AABB2f aabb2{{0.5F, 0.5F}, 0.5F};
  EXPECT_FALSE(Geometry::is_intersecting(aabb1, aabb2));
}

TEST(AABB_Intersect, intersect_exists)
{
  AABB2f aabb1{{0, 0}, 1.0F};
  AABB2f aabb2{{0.5F, 0}, 1.0F};
  AABB2f intersection = Geometry::intersect(aabb1, aabb2);
  AABB2f expectedIntersection{{0.5F, 0}, {1.0F, 1.0F}};
  EXPECT_EQ(intersection, expectedIntersection);
}

TEST(AABB_Intersect, intersect_false)
{
  AABB2f aabb1{{0, 0}, 1.0F};
  AABB2f aabb2{{2.0F, 0}, 1.0F};
  EXPECT_FALSE(Geometry::intersect(aabb1, aabb2).is_valid());
}

TEST(AABB_Intersect, intersect_border_is_touching)
{
  AABB2f aabb1{{0, 0}, 0.5F};
  AABB2f aabb2{{0.5F, 0.0F}, 0.5F};
  EXPECT_FALSE(Geometry::intersect(aabb1, aabb2).is_valid());
}

TEST(AABB_Intersect, intersect_corner_is_touching)
{
  AABB2f aabb1{{0, 0}, 0.5F};
  AABB2f aabb2{{0.5F, 0.5F}, 0.5F};
  EXPECT_FALSE(Geometry::intersect(aabb1, aabb2).is_valid());
}