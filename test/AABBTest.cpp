#include <Geometry/AABB.hpp>
#include <gtest/gtest.h>

#include <linal/vec.hpp>

using namespace Geometry;

class AABBTest3d : public ::testing::Test {
protected:
  AABB3d aabb{{0, 0, 0}, 1.0};
};

TEST(AABBIsValid, default_constructor)
{
  AABB3d const aabb;
  EXPECT_FALSE(aabb.is_valid());
}

TEST_F(AABBTest3d, from_cube_points)
{
  std::vector<linal::double3> const points{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
  AABB3d const box = make_aabb(points);

  linal::double3 const expectedMin{0.0, 0.0, 0.0};
  EXPECT_EQ(box.get_min(), expectedMin);
  linal::double3 const expectedMax{1.0, 1.0, 1.0};
  EXPECT_EQ(box.get_max(), expectedMax);
}

TEST(AABBCenterTest, get_center)
{
  std::vector<linal::double3> const points{{1.0, 1.0, 1.0}, {-1.0, -1.0, -1.0}};
  AABB3d const aabb = make_aabb(points);

  linal::double3 const expectedCenter{0.0, 0.0, 0.0};
  EXPECT_EQ(aabb.center(), expectedCenter);
}

TEST(AABBEmtpyTest, is_empty)
{
  AABB3d const aabb;
  EXPECT_TRUE(aabb.is_empty());
}

TEST(AABBEmtpyTest, is_not_empty)
{
  AABB3d const aabb{{0, 0, 0}, 1.0};
  EXPECT_FALSE(aabb.is_empty());
}

TEST(AABBAddVec, is_inside)
{
  AABB3d const aabb{{0, 0, 0}, 1.0};
  AABB3d modified = aabb;
  modified.add(linal::double3{0.5, 0.5, 0.5});
  EXPECT_EQ(modified, aabb);
}

TEST(AABBAddVec, is_outside)
{
  AABB3d const aabb{{0, 0, 0}, 1.0};
  AABB3d modified = aabb;
  modified.add(linal::double3{1.5, 1.5, 1.5});
  EXPECT_FALSE(modified == aabb);
  AABB3d expectedAABB{{0, 0, 0}, 1.5};
  EXPECT_EQ(modified, expectedAABB);
}

TEST(AABBAddVec, is_on_border)
{
  AABB3d const aabb{{0, 0, 0}, 1.0};
  AABB3d modified = aabb;
  modified.add(linal::double3{1.0, 1.0, 1.0});
  EXPECT_EQ(modified, aabb);
}

TEST(AABBAddAABB, is_inside)
{
  AABB3d const aabb{{0, 0, 0}, 1.0};
  AABB3d modified = aabb;
  AABB3d const other{{0.5, 0.5, 0.5}, 0.1};
  modified.add(other);
  EXPECT_EQ(modified, aabb);
}

TEST(AABBAddAABB, is_outside)
{
  AABB3d const aabb{{0, 0, 0}, 1.0};
  AABB3d modified = aabb;
  AABB3d const other{{1.0, 1.0, 1.0}, 1.0};
  modified.add(other);
  EXPECT_NE(modified, aabb);
  AABB3d expectedAABB{{0, 0, 0}, {2.0, 2.0, 2.0}};
  EXPECT_EQ(modified, expectedAABB);
}