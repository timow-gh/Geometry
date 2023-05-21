#include <Geometry/AABB.hpp>
#include <Geometry/Intersect/IntersectAABB.hpp>
#include <gtest/gtest.h>

using namespace Geometry;

TEST(AABBTest2f_Intersection, is_intersecting)
{
  AABB2f aabb1{{0, 0}, 1.0F};
  AABB2f aabb2{{1.0F, 0}, 1.0F};
  EXPECT_TRUE(Geometry::is_intersecting(aabb1, aabb2));
}

TEST(AABBTest2f_Intersection, borderIsIntersecting)
{
  AABB2f aabb1{{0, 0}, 0.5F};
  AABB2f aabb2{{0.5F, 0.0F}, 0.5F};
  EXPECT_TRUE(Geometry::is_intersecting(aabb1, aabb2));
}

TEST(AABBTest2f_Intersection, notIntersecting)
{
  AABB2f aabb1{{0, 0}, 0.5F};
  AABB2f aabb2{{2.0F, 0}, 0.5F};
  EXPECT_FALSE(Geometry::is_intersecting(aabb1, aabb2));
}

TEST(AABBTest2f_Intersection, corner)
{
  AABB2f aabb1{{0, 0}, 0.5F};
  AABB2f aabb2{{0.5F, 0.5F}, 0.5F};
  EXPECT_TRUE(Geometry::is_intersecting(aabb1, aabb2));
}

TEST(AABBTest3f_Intersection, is_intersecting)
{
  AABB3f aabb1{{0, 0, 0}, 1.0F};
  AABB3f aabb2{{1.0F, 0, 0}, 1.0F};
  EXPECT_TRUE(Geometry::is_intersecting(aabb1, aabb2));
}