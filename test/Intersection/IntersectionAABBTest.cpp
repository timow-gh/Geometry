#include "Geometry/Ray.hpp"
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

// Test AABB - Ray is_intersecting
class AABB_Ray_Intersect : public ::testing::Test {
protected:
  [[nodiscard]] std::array<Ray3f, 3> calc_rays(const linal::float3& origin) const
  {
    return {
        Ray3f{origin, m_directions[0]},
        Ray3f{origin, m_directions[1]},
        Ray3f{origin, m_directions[2]},
    };
  }

  std::array<linal::float3, 3> m_directions{
      linal::float3{1.0F, 0.0F, 0.0F},
      linal::float3{0.0F, 1.0F, 0.0F},
      linal::float3{0.0F, 0.0F, 1.0F},
  };
};

TEST_F(AABB_Ray_Intersect, is_intersecting_ray_inside)
{
  AABB3f aabb{{0, 0, 0}, 1.0F};
  linal::float3 origin{0.5F, 0.5F, 0.5F};
  for (const Ray3f& ray: calc_rays(origin))
  {
    EXPECT_TRUE(Geometry::is_intersecting(aabb, ray));
    EXPECT_TRUE(Geometry::is_intersecting(ray, aabb));
  }
}

TEST_F(AABB_Ray_Intersect, is_intersecting_ray_outside)
{
  AABB3f aabb{{0, 0, 0}, 1.0F};
  linal::float3 origin{0.5F, 0.5F, 0.5F};
  for (const Ray3f& ray: calc_rays(origin))
  {
    EXPECT_TRUE(Geometry::is_intersecting(aabb, ray));
  }
}

TEST_F(AABB_Ray_Intersect, is_intersecting_ray_outside_negative)
{
  AABB3f aabb{{0, 0, 0}, 1.0F};
  linal::float3 origin{-0.5F, -0.5F, -0.5F};
  for (const Ray3f& ray: calc_rays(origin))
  {
    EXPECT_FALSE(Geometry::is_intersecting(aabb, ray));
  }
}

TEST_F(AABB_Ray_Intersect, is_intersecting_ray_touching)
{
  AABB3f aabb{{0, 0, 0}, 1.0F};
  linal::float3 origin{1.0F, 0.5F, 0.5F};
  for (const Ray3f& ray: calc_rays(origin))
  {
    EXPECT_TRUE(Geometry::is_intersecting(aabb, ray));
  }
}

TEST_F(AABB_Ray_Intersect, is_intersecting_ray_touching_negative)
{
  AABB3f aabb{{0, 0, 0}, 1.0F};
  linal::float3 origin{1.0F, 0.5F, 0.5F};
  for (const Ray3f& ray: calc_rays(origin))
  {
    EXPECT_TRUE(Geometry::is_intersecting(ray, aabb));
  }
}

// Test AABB - AABB intersect
TEST_F(AABB_Ray_Intersect, intersect_ray_inside)
{
  AABB3f aabb{{0, 0, 0}, 1.0F};
  linal::float3 origin{0.5F, 0.5F, 0.5F};
  linal::float3 direction{1.0F, 0.0F, 0.0F};
  auto res = Geometry::intersect(aabb, Geometry::Ray3f{origin, direction});
  EXPECT_TRUE(res);
  EXPECT_FALSE(res.has_enter());
  EXPECT_TRUE(res.has_exit());
  linal::float3 expected{1.0F, 0.5F, 0.5F};
  EXPECT_EQ(res.get_exit(), expected);
}

TEST_F(AABB_Ray_Intersect, intersect_ray_outside)
{
  AABB3f aabb{{0, 0, 0}, 1.0F};
  linal::float3 origin{-0.5F, 0.5F, 0.5F};
  linal::float3 direction{1.0F, 0.0F, 0.0F};
  auto res = Geometry::intersect(aabb, Geometry::Ray3f{origin, direction});
  EXPECT_TRUE(res);
  EXPECT_TRUE(res.has_enter());
  linal::float3 expectedEnter{0.0F, 0.5F, 0.5F};
  EXPECT_EQ(res.get_enter(), expectedEnter);
  EXPECT_TRUE(res.has_exit());
  linal::float3 expectedExit{1.0F, 0.5F, 0.5F};
  EXPECT_EQ(res.get_exit(), expectedExit);
}

TEST_F(AABB_Ray_Intersect, intersect_ray_touching)
{
  AABB3f aabb{{0, 0, 0}, 1.0F};
  linal::float3 origin{1.0F, 0.5F, 0.5F};
  linal::float3 direction{1.0F, 0.0F, 0.0F};
  auto res = Geometry::intersect(aabb, Geometry::Ray3f{origin, direction});
  EXPECT_TRUE(res);
  EXPECT_FALSE(res.has_enter());
  EXPECT_TRUE(res.has_exit());
  linal::float3 expected{1.0F, 0.5F, 0.5F};
  EXPECT_EQ(res.get_exit(), expected);
}
