#include <Geometry/Intersect/IntersectLine.hpp>
#include <Geometry/Line.hpp>
#include <gtest/gtest.h>
#include <linal/vec2.hpp>

using namespace Geometry;

TEST(Line2dLine2d, Intersection_OrthogonalA)
{
  Line2d lineA{linal::vec2d{0, 0}, linal::X_VEC2D};
  Line2d lineB{linal::vec2d{0, 0}, linal::Y_VEC2D};
  linal::vec2d intersection;
  uint32_t res = Geometry::intersect(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((linal::vec2d{0, 0}), intersection);
}

TEST(Line2dLine2d, Intersection_OrthogonalB)
{
  Line2d lineA{linal::vec2d{0, 0}, linal::X_VEC2D};
  Line2d lineB{linal::vec2d{1, -1}, linal::Y_VEC2D};
  linal::vec2d intersection;
  uint32_t res = Geometry::intersect(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((linal::vec2d{1, 0}), intersection);
}

TEST(Line2dLine2d, Intersection_OrthogonalC)
{
  Line2d lineA{linal::vec2d{0, 0}, linal::X_VEC2D};
  Line2d lineB{linal::vec2d{1, -1}, linal::Y_VEC2D};
  linal::vec2d intersection;
  uint32_t res = Geometry::intersect(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((linal::vec2d{1, 0}), intersection);
}

TEST(Line2dLine2d, Intersection_SameLine)
{
  Line2d lineA{linal::vec2d{0, 0}, linal::X_VEC2D};
  Line2d lineB{linal::vec2d{1, 0}, linal::X_VEC2D};
  linal::vec2d intersection;
  uint32_t res = Geometry::intersect(lineA, lineB, intersection);
  EXPECT_EQ(res, 2);
}

TEST(Line2dLine2d, No_Intersection)
{
  Line2d lineA{linal::vec2d{0, 1}, linal::X_VEC2D};
  Line2d lineB{linal::vec2d{0, 0}, linal::X_VEC2D};
  linal::vec2d intersection;
  uint32_t res = Geometry::intersect(lineA, lineB, intersection);
  EXPECT_EQ(res, 0);
}

TEST(Line3dLine3d, Skew)
{
  Line3d lineA{linal::vec3d{0, 0, 0}, linal::X_VEC3D};
  Line3d lineB{linal::vec3d{1, -1, 1}, linal::Y_VEC3D};
  linal::vec3d intersection;
  uint32_t res = Geometry::intersect(lineA, lineB, intersection);
  EXPECT_EQ(res, 3);
}

TEST(Line3dLine3d, Intersection_A)
{
  Line3d lineA{linal::vec3d{0, 0, 0}, linal::vec3d{0, 1, 1}};
  Line3d lineB{linal::vec3d{0, 0, 0}, linal::vec3d{1, 0, 1}};
  linal::vec3d intersection;
  uint32_t res = Geometry::intersect(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((linal::vec3d{0, 0, 0}), intersection);
}

TEST(Line3dLine3d, Intersection_Orthogonal_A)
{
  Line3d lineA{linal::vec3d{0, 0, 0}, linal::vec3d{1, 0, 0}};
  Line3d lineB{linal::vec3d{1, -2, 0}, linal::vec3d{0, 1, 0}};
  linal::vec3d intersection;
  uint32_t res = Geometry::intersect(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((linal::vec3d{1, 0, 0}), intersection);
}
