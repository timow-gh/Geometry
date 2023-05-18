#include <Geometry/Cuboid.hpp>
#include <gtest/gtest.h>
#include <linal/vec3.hpp>
#include <linal/utils/constants.hpp>

using namespace Geometry;

class Fixture_Cuboid : public ::testing::Test {
protected:
  Cuboid<double> m_cuboid{linal::ZERO_VEC3D, linal::Z_VEC3D};
};

TEST_F(Fixture_Cuboid, transformation)
{
  auto expectedOrigin = linal::vec3d{0, 0, 0};
  EXPECT_EQ(expectedOrigin, m_cuboid.getOrigin());
}
