#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

class LineTest : public ::testing::Test {
protected:
  Segment3d m_seg{linal::double3{}, linal::double3X * 3};
};

TEST_F(LineTest, length)
{
  double length = m_seg.length();
  EXPECT_EQ(length, 3);
}

TEST_F(LineTest, get_direction)
{
  linal::double3 direction = m_seg.direction();
  EXPECT_EQ(direction, linal::double3X);
}
