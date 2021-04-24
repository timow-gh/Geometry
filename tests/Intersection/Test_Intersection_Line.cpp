#pragma clang diagnostic push12 / 1043
#pragma ide diagnostic ignored "cert-err58-cpp"

#include <iostream>

#include "gtest/gtest.h"

#include "LinAl/LinearAlgebra.hpp"

#include "Geometry/IntersectionLine.hpp"
#include "Geometry/Line.hpp"

using namespace Geometry;
using namespace LinAl;

TEST(Line2dLine2d, Intersection_OrthogonalA) {
  Line2d lineA{Vec2d{0, 0}, X_VEC2D};
  Line2d lineB{Vec2d{0, 0}, Y_VEC2D};
  Vec2d intersection;
  uint32_t res = calcIntersection(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((Vec2d{0, 0}), intersection);
}

TEST(Line2dLine2d, Intersection_OrthogonalB) {
  Line2d lineA{Vec2d{0, 0}, X_VEC2D};
  Line2d lineB{Vec2d{1, -1}, Y_VEC2D};
  Vec2d intersection;
  uint32_t res = calcIntersection(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((Vec2d{1, 0}), intersection);
}

TEST(Line2dLine2d, Intersection_OrthogonalC) {
  Line2d lineA{Vec2d{0, 0}, X_VEC2D};
  Line2d lineB{Vec2d{1, -1}, Y_VEC2D};
  Vec2d intersection;
  uint32_t res = calcIntersection(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((Vec2d{1, 0}), intersection);
}

TEST(Line2dLine2d, Intersection_SameLine) {
  Line2d lineA{Vec2d{0, 0}, X_VEC2D};
  Line2d lineB{Vec2d{1, 0}, X_VEC2D};
  Vec2d intersection;
  uint32_t res = calcIntersection(lineA, lineB, intersection);
  EXPECT_EQ(res, 2);
}

TEST(Line2dLine2d, No_Intersection) {
  Line2d lineA{Vec2d{0, 1}, X_VEC2D};
  Line2d lineB{Vec2d{0, 0}, X_VEC2D};
  Vec2d intersection;
  uint32_t res = calcIntersection(lineA, lineB, intersection);
  EXPECT_EQ(res, 0);
}

TEST(Line3dLine3d, Skew) {
  Line3d lineA{Vec3d{0, 0, 0}, X_VEC3D};
  Line3d lineB{Vec3d{1, -1, 1}, Y_VEC3D};
  Vec3d intersection;
  uint32_t res = calcIntersection(lineA, lineB, intersection);
  EXPECT_EQ(res, 3);
}

TEST(Line3dLine3d, Intersection_A) {
  Line3d lineA{Vec3d{0, 0, 0}, Vec3d{0, 1, 1}};
  Line3d lineB{Vec3d{0, 0, 0}, Vec3d{1, 0, 1}};
  Vec3d intersection;
  uint32_t res = calcIntersection(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((Vec3d{0, 0, 0}), intersection);
}

TEST(Line3dLine3d, Intersection_Orthogonal_A) {
  Line3d lineA{Vec3d{0, 0, 0}, Vec3d{1, 0, 0}};
  Line3d lineB{Vec3d{1, -2, 0}, Vec3d{0, 1, 0}};
  Vec3d intersection;
  uint32_t res = calcIntersection(lineA, lineB, intersection);
  EXPECT_EQ(res, 1);
  EXPECT_EQ((Vec3d{1, 0, 0}), intersection);
}

#pragma clang diagnostic pop