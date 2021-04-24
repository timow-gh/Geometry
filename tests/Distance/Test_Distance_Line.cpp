#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <iostream>

#include "gtest/gtest.h"

#include "LinAl/LinearAlgebra.hpp"

#include "Geometry/Line.hpp"

using namespace Geometry;
using namespace LinAl;

TEST(Vec_Line_Distance, DistTestA) {
  Vec3d vec{2, 2, 0};
  Line3d line{ZERO_VEC3D, X_VEC3D};
  double_t dist = distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST(Vec_Line_Distance, DistTestB) {
  Vec3d vec{0, 2, 2};
  Line3d line{ZERO_VEC3D, X_VEC3D};
  double_t dist = distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, norm2(Vec2d{2, 2}));
}

TEST(Vec_Line_Distance, DistTestC) {
  Vec3d vec{0, 0, 0};
  Line3d line{Vec3d{1, 0, 0}, Z_VEC3D};
  double_t dist = distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, 1);
}

TEST(Vec_Line_Distance, OnLineTest) {
  Vec3d vec{0, 0, 0};
  Line3d line{ZERO_VEC3D, X_VEC3D};
  double_t dist = distance(vec, line);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST(Vec_Line_Distance, MemberDistance) {
  Line3d line{ZERO_VEC3D, X_VEC3D};
  Vec3d vec{0, 3, 0};
  double_t dist = line.distance(vec);
  EXPECT_DOUBLE_EQ(dist, 3);
}

TEST(Line_Line_Distance, distance) {
  Line3d lhs{ZERO_VEC3D, Y_VEC3D};
  Line3d rhs{Vec3d{0, 0, 2}, X_VEC3D};
  double_t dist = distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST(Line_Line_Distance, collinear_parallel) {
  Line3d lhs{ZERO_VEC3D, X_VEC3D};
  Line3d rhs{Vec3d{0, 0, 3}, X_VEC3D};
  double_t dist = distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 3);
}

TEST(Line_Line_Distance, parallel_lines) {
  Line3d lhs{ZERO_VEC3D, X_VEC3D};
  Line3d rhs{ZERO_VEC3D, X_VEC3D};
  double_t dist = distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST(Line_Line_Distance, linesWithIntersection) {
  Line3d lhs{ZERO_VEC3D, Y_VEC3D};
  Line3d rhs{ZERO_VEC3D, X_VEC3D};
  double_t dist = distance(lhs, rhs);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST(Line_Line_Distance, memberFunction) {
  Line3d lhs{ZERO_VEC3D, Y_VEC3D};
  Line3d rhs{Vec3d{0, 0, 5}, X_VEC3D};
  double_t dist = lhs.distance(rhs);
  EXPECT_DOUBLE_EQ(dist, 5);
}

#pragma clang diagnostic pop