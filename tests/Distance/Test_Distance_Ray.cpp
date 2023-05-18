#include <Core/Types/TVector.hpp>
#include <Geometry/Distance/DistanceRay.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Ray.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>

using namespace Geometry;
using namespace LinAl;

class Distance_Ray_Test : public ::testing::Test {
  protected:
    Vec3d source{1, 1, 1};
    Ray3d ray{source, X_VEC3D};
};

TEST_F(Distance_Ray_Test, PointAtRayStart)
{
    double dist = Geometry::distance(ray, source);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST_F(Distance_Ray_Test, PointAtRayStart_ArgOrder)
{
    double dist = distance(source, ray);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST(Distance_Ray, OnLine_UnitDist)
{
    Vec3d source{1, 0, 0};
    Ray3d ray{source, X_VEC3D};
    double dist = Geometry::distance(ray, ZERO_VEC3D);
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

TEST(Distance_Ray, OnLine_ZeroDist)
{
    Vec3d source{1, 0, 0};
    Ray3d ray{source, X_VEC3D};
    double dist = Geometry::distance(ray, Vec3d{2, 0, 0});
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST(Distance_Ray, NextToLine_UnitDist)
{
    Ray3d ray{Vec3d{1, 0, 0}, X_VEC3D};
    double dist = Geometry::distance(ray, Vec3d{2, 1, 0});
    EXPECT_DOUBLE_EQ(dist, 1.0);
}

class Test_Ray_Creator : public ::testing::Test {
  protected:
    Core::TVector<Ray3d> m_ray3ds;

    void SetUp() override
    {
        m_ray3ds.push_back(Ray(LinAl::Vec3d{-1, 0, 0}, LinAl::X_VEC3D));
        m_ray3ds.push_back(Ray(LinAl::Vec3d{}, LinAl::Y_VEC3D));
        m_ray3ds.push_back(Ray(LinAl::Vec3d{}, LinAl::ZERO_VEC3D));
    }
};

TEST_F(Test_Ray_Creator, RayTestFixtureTest)
{
    Core::TVector<Ray3d> ray3ds;
    for (const auto& ray: m_ray3ds)
        ray3ds.push_back(ray);
    EXPECT_EQ(ray3ds.size(), 3);
}

class RayDistanceTestFixture : public ::testing::Test {
  protected:
    Ray3d m_ray3d{Vec3d{}, LinAl::X_VEC3D};
};

TEST_F(RayDistanceTestFixture, PointOnRaySide)
{
    LinAl::Vec3d vec{1, 2, 0};
    double dist = Geometry::distance(m_ray3d, vec);
    EXPECT_DOUBLE_EQ(dist, 2);
}

TEST_F(RayDistanceTestFixture, PointOrthogonalAtRayOrigin)
{
    LinAl::Vec3d vec{0, 3, 0};
    double dist = Geometry::distance(m_ray3d, vec);
    EXPECT_DOUBLE_EQ(dist, 3);
}

TEST_F(RayDistanceTestFixture, PointBeforeRayOrigin)
{
    LinAl::Vec3d vec{-1, 3, 0};
    double dist = Geometry::distance(m_ray3d, vec);
    EXPECT_DOUBLE_EQ(dist, LinAl::norm2(vec));
}
