#include <Geometry/PolyLine.hpp>
#include <gtest/gtest.h>
#include <linal/containers.hpp>
#include <linal/vec3.hpp>

using namespace Geometry;

TEST(PolyLine, Ctor)
{
  linal::vec3d first = {0, 0, 0};
  linal::vec3d second = {0, 0, 0};

  linal::vec3dvector points = {first, second};
  PolyLine3d polyLine = PolyLine3d(points);
  linal::vec3dvector polyLinePoints = polyLine.getPoints();

  EXPECT_EQ(first, polyLinePoints[0]);
  EXPECT_EQ(second, polyLinePoints[1]);
}
