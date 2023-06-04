#include <Geometry/Distance/DistanceLine.hpp>
#include <Geometry/Line.hpp>
#include <gtest/gtest.h>
#include <linal/vec2.hpp>
#include <linal/vec3.hpp>

using namespace Geometry;

class Vec_Line_Distance_Test : public ::testing::Test {
protected:
  linal::vec3d vec{2, 2, 0};
  Line3d line{linal::ZERO_VEC3D, linal::X_VEC3D};
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
  linal::vec3d vec{0, 2, 2};
  Line3d line{linal::ZERO_VEC3D, linal::X_VEC3D};
  double dist = Geometry::distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, linal::norm2(linal::vec2d{2, 2}));
}

TEST(Vec_Line_Distance, DistTestC)
{
  linal::vec3d vec{0, 0, 0};
  Line3d line{linal::vec3d{1, 0, 0}, linal::Z_VEC3D};
  double dist = Geometry::distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, 1);
}

TEST(Vec_Line_Distance, OnLineTest)
{
  linal::vec3d vec{0, 0, 0};
  Line3d line{linal::ZERO_VEC3D, linal::X_VEC3D};
  double dist = Geometry::distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST(Line_Line_Distance, distance)
{
  Line3d lhs{linal::ZERO_VEC3D, linal::Y_VEC3D};
  Line3d rhs{linal::vec3d{0, 0, 2}, linal::X_VEC3D};
  double dist = Geometry::distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST(Line_Line_Distance, collinear_parallel)
{
  Line3d lhs{linal::ZERO_VEC3D, linal::X_VEC3D};
  Line3d rhs{linal::vec3d{0, 0, 3}, linal::X_VEC3D};
  double dist = Geometry::distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 3);
}

TEST(Line_Line_Distance, parallel_lines)
{
  Line3d lhs{linal::ZERO_VEC3D, linal::X_VEC3D};
  Line3d rhs{linal::ZERO_VEC3D, linal::X_VEC3D};
  double dist = Geometry::distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST(Line_Line_Distance, linesWithIntersection)
{
  Line3d lhs{linal::ZERO_VEC3D, linal::Y_VEC3D};
  Line3d rhs{linal::ZERO_VEC3D, linal::X_VEC3D};
  double dist = Geometry::distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 0);
}
