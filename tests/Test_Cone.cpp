#include <linal/utils/constants.hpp>
#include <Geometry/Cone.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

class Fixture_Cone : public ::testing::Test {
protected:
  Cone<double> m_cone{Segment3d{linal::double3{}, linal::double3Z}, 1.0};
};

TEST_F(Fixture_Cone, height)
{
  auto height = m_cone.get_height();
  EXPECT_DOUBLE_EQ(height, 1.0);
}

TEST_F(Fixture_Cone, slantHeight)
{
  auto slantHeight = m_cone.get_slant_height();
  EXPECT_DOUBLE_EQ(slantHeight, std::sqrt(2.0));
}

TEST_F(Fixture_Cone, openingAngle)
{
  auto openingAngle = m_cone.get_opening_angle();
  // rad = 90 * linal::PI / 180
  EXPECT_DOUBLE_EQ(openingAngle, 90 * linal::PI_D / 180);
}
