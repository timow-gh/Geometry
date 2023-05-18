#include <Geometry/PolyLine.hpp>
#include <gtest/gtest.h>
#include <linal/Containers.hpp>
#include <linal/Vec3.hpp>

using namespace Geometry;

TEST(PolyLine, Ctor)
{
  linal::Vec3d first = {0, 0, 0};
  linal::Vec3d second = {0, 0, 0};

  linal::Vec3dVector points = {first, second};
  PolyLine3d polyLine = PolyLine3d(points);
  linal::Vec3dVector polyLinePoints = polyLine.getPoints();

  EXPECT_EQ(first, polyLinePoints[0]);
  EXPECT_EQ(second, polyLinePoints[1]);
}
