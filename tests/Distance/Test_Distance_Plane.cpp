#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "gtest/gtest.h"

#include "LinAl/LinearAlgebra.hpp"

#include "Geometry/DistancePlane.hpp"
#include "Geometry/Plane.hpp"

using namespace Geometry;
using namespace LinAl;

class PlaneDistanceTestFixture : public ::testing::Test {
protected:
  Plane<double_t> m_plane{Vec3d{}, LinAl::X_VEC3D};
};

TEST_F(PlaneDistanceTestFixture, PointDistanceA) {
  LinAl::Vec3d point{2, 0, 0};
  double_t dist = distance(m_plane, point);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(PlaneDistanceTestFixture, PointDistanceB) {
  LinAl::Vec3d point{-2, 0, 0};
  double_t dist = distance(m_plane, point);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(PlaneDistanceTestFixture, PointDistanceC) {
  LinAl::Vec3d point{9, 4, -2};
  double_t dist = distance(m_plane, point);
  EXPECT_DOUBLE_EQ(dist, 9);
}

TEST_F(PlaneDistanceTestFixture, PointInPlane) {
  LinAl::Vec3d point{0, 4, 0};
  double_t dist = distance(m_plane, point);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(PlaneDistanceTestFixture, memberFunction) {
  LinAl::Vec3d point{3, 0, 0};
  double_t dist = m_plane.distance(point);
  EXPECT_DOUBLE_EQ(dist, 3);
}

#pragma clang diagnostic pop