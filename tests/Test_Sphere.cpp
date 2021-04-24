#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "gtest/gtest.h"

#include "Geometry/Sphere.hpp"

using namespace Geometry;
using namespace LinAl;

TEST(Sphere, Constructor) {
  constexpr double_t RADIUS = 1;
  Sphere sphere{ZERO_VEC3D, RADIUS};
  EXPECT_EQ(sphere.getRadius(), RADIUS);
  EXPECT_EQ(sphere.getOrigin(), ZERO_VEC3D);
}

TEST(Sphere, Equality) {
  constexpr double_t RADIUS = 1;
  Sphere sphereA{ZERO_VEC3D, RADIUS};
  Sphere sphereB{ZERO_VEC3D, RADIUS};
  EXPECT_EQ(sphereA, sphereB);
}

TEST(Sphere, Inequality) {
  constexpr double_t RADIUS_A = 1;
  Sphere sphereA{ZERO_VEC3D, RADIUS_A};
  constexpr double_t RADIUS_B = 2;
  Sphere sphereB{ZERO_VEC3D, RADIUS_B};
  EXPECT_NE(sphereA, sphereB);
}

#pragma clang diagnostic pop