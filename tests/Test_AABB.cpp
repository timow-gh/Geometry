#include <Geometry/AABB.hpp>
#include <gtest/gtest.h>
#include <linal/containers.hpp>
#include <linal/vec3.hpp>

using namespace Geometry;

class AABBTest3d : public ::testing::Test {
protected:
  AABB3d aabb{{0, 0, 0}, 1.0};
};

TEST_F(AABBTest3d, fromcubepoints)
{
  linal::vec3dvector const points{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
  AABB3d const box = makeAABB(points);

  linal::vec3d const expectedMin{0.0, 0.0, 0.0};
  EXPECT_EQ(box.get_min(), expectedMin);
  linal::vec3d const expectedMax{1.0, 1.0, 1.0};
  EXPECT_EQ(box.get_max(), expectedMax);
}
