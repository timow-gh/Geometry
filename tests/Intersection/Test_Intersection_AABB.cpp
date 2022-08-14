#include <Geometry/AABB.hpp>
#include <Geometry/Intersection/IntersectionAABB.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

TEST(AABBTest2f_Intersection, isIntersecting)
{
    AABB2f aabb1{{0, 0}, 1.0f};
    AABB2f aabb2{{1.0f, 0}, 1.0f};
    EXPECT_TRUE(Geometry::isIntersecting(aabb1, aabb2));
}

TEST(AABBTest2f_Intersection, borderIsIntersecting)
{
    AABB2f aabb1{{0, 0}, 0.5f};
    AABB2f aabb2{{1.0f, 0}, 0.5f};
    EXPECT_TRUE(Geometry::isIntersecting(aabb1, aabb2));
}

TEST(AABBTest2f_Intersection, notIntersecting)
{
    AABB2f aabb1{{0, 0}, 0.5f};
    AABB2f aabb2{{2.0f, 0}, 0.5f};
    EXPECT_FALSE(Geometry::isIntersecting(aabb1, aabb2));
}

TEST(AABBTest2f_Intersection, corner)
{
    AABB2f aabb1{{0, 0}, 0.5f};
    AABB2f aabb2{{1.0f, 1.0f}, 0.5f};
    EXPECT_TRUE(Geometry::isIntersecting(aabb1, aabb2));
}