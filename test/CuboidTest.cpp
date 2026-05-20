#include <Geometry/Cuboid.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>
#include <linal/utils/constants.hpp>

using namespace Geometry;

class Fixture_Cuboid : public ::testing::Test {
protected:
  Cuboid<double> m_cuboid{linal::double3{}, linal::double3Z};
};

TEST_F(Fixture_Cuboid, transformation)
{
  auto expectedOrigin = linal::double3{0, 0, 0};
  EXPECT_EQ(expectedOrigin, m_cuboid.get_origin());
}
