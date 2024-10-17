#include <Geometry/Distance/DistanceLine.hpp>
#include <Geometry/Line.hpp>
#include <gtest/gtest.h>
#include <linal/vec.hpp>

using namespace Geometry;

class Vec_Line_Distance_Test : public ::testing::Test {
protected:
  linal::double3 vec{2, 2, 0};
  Line3d line{linal::double3{}, linal::double3X};
};

TEST_F(Vec_Line_Distance_Test, DistTestA)
{
  double dist = Geometry::distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(Vec_Line_Distance_Test, DistTestA_ArgOrder)
{
  double dist = Geometry::distance(line, vec);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST(Vec_Line_Distance, DistTestB)
{
  linal::double3 vec{0, 2, 2};
  Line3d line{linal::double3{}, linal::double3X};
  double dist = Geometry::distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, linal::length(linal::double2{2, 2}));
}

TEST(Vec_Line_Distance, DistTestC)
{
  linal::double3 vec{0, 0, 0};
  Line3d line{linal::double3{1, 0, 0}, linal::double3Z};
  double dist = Geometry::distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, 1);
}

TEST(Vec_Line_Distance, OnLineTest)
{
  linal::double3 vec{0, 0, 0};
  Line3d line{linal::double3{}, linal::double3X};
  double dist = Geometry::distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST(Line_Line_Distance, distance)
{
  Line3d lhs{linal::double3{}, linal::double3Y};
  Line3d rhs{linal::double3{0, 0, 2}, linal::double3X};
  double dist = Geometry::distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST(Line_Line_Distance, collinear_parallel)
{
  Line3d lhs{linal::double3{}, linal::double3X};
  Line3d rhs{linal::double3{0, 0, 3}, linal::double3X};
  double dist = Geometry::distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 3);
}

TEST(Line_Line_Distance, parallel_lines)
{
  Line3d lhs{linal::double3{}, linal::double3X};
  Line3d rhs{linal::double3{}, linal::double3X};
  double dist = Geometry::distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST(Line_Line_Distance, linesWithIntersection)
{
  Line3d lhs{linal::double3{}, linal::double3Y};
  Line3d rhs{linal::double3{}, linal::double3X};
  double dist = Geometry::distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST(Line_Line_Distance, negativeDistTest)
{
  Line3d lhs{linal::double3{0.0, 0.0, -1.0}, linal::double3{0.0, 1.0, 0.0}};
  Line3d rhs{linal::double3{0.0, 0.0, 0.0}, linal::double3X};
  double dist = Geometry::distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 1);
}
