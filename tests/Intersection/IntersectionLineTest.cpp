#include <Geometry/Intersect/IntersectLine.hpp>
#include <Geometry/Line.hpp>
#include <gtest/gtest.h>


using namespace Geometry;

TEST(Line2dLine2d, Intersection_OrthogonalA)
{
  Line2d lineA{linal::double2{0, 0}, linal::double2X};
  Line2d lineB{linal::double2{0, 0}, linal::double2Y};

  auto res = Geometry::intersect(lineA, lineB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double2{0, 0}), *res);
}

TEST(Line2dLine2d, Intersection_OrthogonalB)
{
  Line2d lineA{linal::double2{0, 0}, linal::double2X};
  Line2d lineB{linal::double2{1, -1}, linal::double2Y};

  auto res = Geometry::intersect(lineA, lineB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double2{1, 0}), *res);
}

TEST(Line2dLine2d, Intersection_OrthogonalC)
{
  Line2d lineA{linal::double2{0, 0}, linal::double2X};
  Line2d lineB{linal::double2{1, -1}, linal::double2Y};

  auto res = Geometry::intersect(lineA, lineB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double2{1, 0}), *res);
}

TEST(Line2dLine2d, Intersection_SameLine)
{
  Line2d lineA{linal::double2{0, 0}, linal::double2X};
  Line2d lineB{linal::double2{1, 0}, linal::double2X};

  auto res = Geometry::intersect(lineA, lineB);
  EXPECT_FALSE(res);
}

TEST(Line2dLine2d, No_Intersection)
{
  Line2d lineA{linal::double2{0, 1}, linal::double2X};
  Line2d lineB{linal::double2{0, 0}, linal::double2X};

  auto res = Geometry::intersect(lineA, lineB);
  EXPECT_FALSE(res);
}

TEST(Line3dLine3d, Skew)
{
  Line3d lineA{linal::double3{0, 0, 0}, linal::double3X};
  Line3d lineB{linal::double3{1, -1, 1}, linal::double3Y};

  auto res = Geometry::intersect(lineA, lineB);
  EXPECT_FALSE(res);
}

TEST(Line3dLine3d, Intersection_A)
{
  Line3d lineA{linal::double3{0, 0, 0}, linal::double3{0, 1, 1}};
  Line3d lineB{linal::double3{0, 0, 0}, linal::double3{1, 0, 1}};

  auto res = Geometry::intersect(lineA, lineB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{0, 0, 0}), *res);
}

TEST(Line3dLine3d, Intersection_Orthogonal_A)
{
  Line3d lineA{linal::double3{0, 0, 0}, linal::double3{1, 0, 0}};
  Line3d lineB{linal::double3{1, -2, 0}, linal::double3{0, 1, 0}};

  auto res = Geometry::intersect(lineA, lineB);
  EXPECT_TRUE(res);
  EXPECT_EQ((linal::double3{1, 0, 0}), *res);
}
