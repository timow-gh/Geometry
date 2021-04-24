#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "gtest/gtest.h"

#include "LinAl/LinearAlgebra.hpp"

#include "Geometry/DistanceSegment.hpp"
#include "Geometry/Segment.hpp"

using namespace Geometry;
using namespace LinAl;

class Segment_VecDistanceTestFixture : public ::testing::Test {
protected:
  Segment3d m_segment3d{Vec3d{}, LinAl::X_VEC3D};
};

TEST_F(Segment_VecDistanceTestFixture, PointAtSegSource) {
  LinAl::Vec3d point{0};
  double_t dist = distance(m_segment3d, point);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistanceTestFixture, PointAtSegTarget) {
  LinAl::Vec3d point = LinAl::Vec3d{1, 0, 0};
  double_t dist = distance(m_segment3d, point);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistanceTestFixture, PointUnitDistAtSegSource) {
  LinAl::Vec3d point{0, 1, 0};
  double_t dist = distance(m_segment3d, point);
  EXPECT_DOUBLE_EQ(dist, 1);
}

TEST_F(Segment_VecDistanceTestFixture, PointUnitDistAtSegTarget) {
  LinAl::Vec3d point{1, 1, 0};
  double_t dist = distance(m_segment3d, point);
  EXPECT_DOUBLE_EQ(dist, 1);
}

TEST_F(Segment_VecDistanceTestFixture, PointAtSegMidPoint) {
  LinAl::Vec3d point{0.5, 0, 0};
  double_t dist = distance(m_segment3d, point);
  EXPECT_DOUBLE_EQ(dist, 0);
}

TEST_F(Segment_VecDistanceTestFixture, PointCollinearBeforeSegment) {
  LinAl::Vec3d point{-3, 0, 0};
  double_t dist = distance(m_segment3d, point);
  EXPECT_DOUBLE_EQ(dist, 3);
}

TEST_F(Segment_VecDistanceTestFixture, PointCollinearAfterSegment) {
  LinAl::Vec3d point{3, 0, 0};
  double_t dist = distance(m_segment3d, point);
  EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(Segment_VecDistanceTestFixture, PointOffsetAtSegmentMidPoint) {
  LinAl::Vec3d point{0.5, 3, 0};
  double_t dist = distance(m_segment3d, point);
  EXPECT_DOUBLE_EQ(dist, 3);

  point = LinAl::Vec3d{0.5, -3, 0};
  dist = distance(m_segment3d, point);
  EXPECT_DOUBLE_EQ(dist, 3);
}

TEST_F(Segment_VecDistanceTestFixture, memberFunction) {
  LinAl::Vec3d point{0.5, 3, 0};
  auto dist = m_segment3d.distance(point);
  EXPECT_DOUBLE_EQ(dist, 3);
}

#pragma clang diagnostic pop