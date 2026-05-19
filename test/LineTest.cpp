#include <Geometry/Line.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

class LineTest : public ::testing::Test {
protected:
  Line3d m_line{linal::double3{}, linal::double3X * 2};
};

TEST_F(LineTest, get_direction)
{
  linal::double3 direction = m_line.get_direction();
  EXPECT_EQ(direction, linal::double3X);
}
