#include <Geometry/Circle.hpp>
#include <gtest/gtest.h>
#include <linal/vec3.hpp>

using namespace Geometry;

class Circle3d_xz : public ::testing::Test {
protected:
  // Circle lies in the xy plane
  Circle3d circle{linal::vec3d{0, 0, 0}, 1.0, linal::vec3d{0, -1.0, 0}};
};

TEST_F(Circle3d_xz, circleOrigin)
{
  linal::vec3d expectedOrigin{0, 0, 0};
  EXPECT_EQ(circle.get_origin(), expectedOrigin);
}

class Circle3d_Translation : public ::testing::Test {
protected:
  // Circle lies in the xy plane
  Circle3d circle{linal::vec3d{10, 3, 4}, 1.0, linal::vec3d{1.0, 0, 0}};
};
