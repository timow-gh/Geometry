#include <Geometry/Plane.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

class LineTest : public ::testing::Test {
protected:
  Plane<double> m_plane{linal::double3{}, linal::double3Z * 2};
};

TEST_F(LineTest, get_normal)
{
  linal::double3 normal = m_plane.get_normal();
  EXPECT_EQ(normal, linal::double3Z);
}
