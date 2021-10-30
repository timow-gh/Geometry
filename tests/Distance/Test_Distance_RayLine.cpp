#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "Core/Types/TVector.hpp"
#include "Geometry/Line.hpp"
#include "Geometry/Ray.hpp"
#include "LinAl/LinearAlgebra.hpp"
#include "gtest/gtest.h"

using namespace Geometry;
using namespace LinAl;

class RayLineDistanceTests
    : public ::testing::Test
{
  protected:
    Ray3d m_ray3d{Vec3d{1, 1, 1}, LinAl::X_VEC3D};
};

TEST_F(RayLineDistanceTests, LineThroughRayOrigin)
{
    auto line = Line3<double_t>{LinAl::ZERO_VEC3D, LinAl::Vec3d{1, 1, 1}};
    double_t dist = distance(m_ray3d, line);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(RayLineDistanceTests, LineDistOne)
{
    auto line = Line3<double_t>{LinAl::ZERO_VEC3D, LinAl::Vec3d{1, 0, 1}};
    double_t dist = distance(m_ray3d, line);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(RayLineDistanceTests, LinePerpendicularToRay)
{
    auto line = Line3<double_t>{LinAl::ZERO_VEC3D, LinAl::Vec3d{0, 0, 1}};
    m_ray3d = Ray3d{Vec3d{0, 1, 0}, LinAl::X_VEC3D};
    double_t dist = distance(m_ray3d, line);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(RayLineDistanceTests, IntersectingLineRay)
{
    auto line = Line3<double_t>{Vec3d{1, 0, -1}, LinAl::Vec3d{0, 0, 1}};
    m_ray3d =  Ray3d{Vec3d{0, 0, 0}, LinAl::X_VEC3D};
    double_t dist = distance(m_ray3d, line);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(RayLineDistanceTests, SkewLineToRay)
{
    auto line = Line3<double_t>{Vec3d{2, 1, 1}, LinAl::Vec3d{1,-1,1}};
    m_ray3d =  Ray3d{Vec3d{0,0,0}, LinAl::Vec3d{1,0,0}};
    double_t dist = distance(m_ray3d, line);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(RayLineDistanceTests, ParallelLineRay)
{
    auto line = Line3<double_t>{Vec3d{0,0,1}, LinAl::Vec3d{1,0,0}};
    m_ray3d =  Ray3d{Vec3d{0, 0, 0}, LinAl::Vec3d{1,0,0}};
    double_t dist = distance(m_ray3d, line);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST_F(RayLineDistanceTests, ParameterOrder_LineRay)
{
    auto line = Line3<double_t>{Vec3d{0,0,1}, LinAl::Vec3d{1,0,0}};
    m_ray3d =  Ray3d{Vec3d{0, 0, 0}, LinAl::Y_VEC3D};
    double_t dist = distance(line, m_ray3d);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

#pragma clang diagnostic pop