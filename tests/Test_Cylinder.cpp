#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "gtest/gtest.h"

#include "Geometry/Cylinder.hpp"

using namespace Geometry;
using namespace LinAl;

TEST(Cylinder, Constructor) {
  constexpr double_t HEIGHT = 5;
  constexpr double_t RADIUS = 1;
  const auto originLcs = Lcs<double_t>();
  Cylinder cylinder{originLcs, HEIGHT, RADIUS};
  EXPECT_EQ(cylinder.getOriginLcs(), originLcs);
  EXPECT_EQ(cylinder.getHeight(), HEIGHT);
  EXPECT_EQ(cylinder.getRadius(), RADIUS);
}

TEST(Cylinder, Equality) {
  constexpr double_t HEIGHT = 5;
  constexpr double_t RADIUS = 1;
  const auto originLcs = Lcs<double_t>();
  Cylinder cylinderA{originLcs, HEIGHT, RADIUS};
  Cylinder cylinderB{originLcs, HEIGHT, RADIUS};
  EXPECT_EQ(cylinderA, cylinderB);
}

TEST(Cylinder, Inequality) {
  constexpr double_t HEIGHT = 5;
  constexpr double_t RADIUS_A = 1;
  constexpr double_t RADIUS_B = 2;
  const auto originLcs = Lcs<double_t>();
  Cylinder cylinderA{originLcs, HEIGHT, RADIUS_A};
  Cylinder cylinderB{originLcs, HEIGHT, RADIUS_B};
  EXPECT_NE(cylinderA, cylinderB);
}

#pragma clang diagnostic pop