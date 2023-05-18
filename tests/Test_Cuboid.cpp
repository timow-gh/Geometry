#include <Core/Math/Constants.hpp>
#include <Geometry/Cuboid.hpp>
#include <gtest/gtest.h>
#include <linal/Vec3.hpp>

using namespace Geometry;

class Fixture_Cuboid : public ::testing::Test {
protected:
  Cuboid<double> m_cuboid{linal::ZERO_VEC3D, linal::Z_VEC3D};
};

TEST_F(Fixture_Cuboid, transformation)
{
  auto expectedOrigin = linal::Vec3d{0, 0, 0};
  EXPECT_EQ(expectedOrigin, m_cuboid.getOrigin());
}
