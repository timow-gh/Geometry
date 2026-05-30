#include <Geometry/Ray.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

class LineTest : public ::testing::Test {
protected:
  Ray3d m_ray{linal::double3{}, linal::double3X * 2};
};

TEST_F(LineTest, get_direction)
{
  linal::double3 direction = m_ray.get_direction();
  EXPECT_EQ(direction, linal::double3X);
}

TEST(RayTest, set_direction_normalizes)
{
  Ray3d ray{linal::double3{}, linal::double3X};
  ray.set_direction(linal::double3Y * 2);
  EXPECT_EQ(ray.get_direction(), linal::double3Y);
}
