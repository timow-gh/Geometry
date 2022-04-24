#include <Geometry/Intersection/IntersectionSphere.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Segment.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>
#include <optional>

using namespace LinAl;
using namespace Geometry;

TEST(Sphere_Line3d, Intersection)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Line3d line{ZERO_VEC3D, X_VEC3D};
    auto sphereIntersec = calcIntersection(sphere, line);
    EXPECT_TRUE(sphereIntersec.first);
    EXPECT_TRUE(sphereIntersec.second);
    EXPECT_EQ(sphereIntersec.first.value(), X_VEC3D);
    EXPECT_EQ(sphereIntersec.second.value(), -X_VEC3D);
}

TEST(Sphere_Line3d, Touching)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Line3d line{Z_VEC3D, X_VEC3D};
    auto sphereIntersec = calcIntersection(sphere, line);
    EXPECT_EQ(sphereIntersec.first.value(), Z_VEC3D);
    EXPECT_FALSE(sphereIntersec.second);
}

TEST(Sphere_Line3d, No_Intersection)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Line3d line{Vec3d{0, 0, 2}, X_VEC3D};
    auto sphereIntersec = calcIntersection(sphere, line);
    EXPECT_FALSE(sphereIntersec.hasIntersection());
}

TEST(Sphere_Ray3d, Intersection)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Ray3d ray{Vec3d{-3, 0, 0}, X_VEC3D};
    auto sphereIntersec = calcIntersection(sphere, ray);
    EXPECT_TRUE(sphereIntersec.first);
    EXPECT_TRUE(sphereIntersec.second);
    EXPECT_EQ(sphereIntersec.first.value(), X_VEC3D);
    EXPECT_EQ(sphereIntersec.second.value(), -X_VEC3D);
}

TEST(Sphere_Ray3d, Touching)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Ray3d ray{Z_VEC3D, X_VEC3D};
    auto sphereIntersec = calcIntersection(sphere, ray);
    EXPECT_TRUE(sphereIntersec.first);
    EXPECT_FALSE(sphereIntersec.second);
    EXPECT_EQ(sphereIntersec.first.value(), Z_VEC3D);
}

TEST(Sphere_Ray3d, InsideSphere)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Ray3d ray{ZERO_VEC3D, X_VEC3D};
    auto sphereIntersec = calcIntersection(sphere, ray);
    EXPECT_TRUE(sphereIntersec.first);
    EXPECT_FALSE(sphereIntersec.second);
    EXPECT_EQ(sphereIntersec.first.value(), X_VEC3D);
}

TEST(Sphere_Ray3d, No_Intersection)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Ray3d ray{Vec3d{0, 3, 0}, X_VEC3D};
    auto sphereIntersec = calcIntersection(sphere, ray);
    EXPECT_FALSE(sphereIntersec.hasIntersection());
}

TEST(Sphere_Segment3d, Intersection)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Segment3d seg{Vec3d{-2, 0, 0}, Vec3d{2, 0, 0}};
    auto sphereIntersec = calcIntersection(sphere, seg);
    EXPECT_TRUE(sphereIntersec.first);
    EXPECT_TRUE(sphereIntersec.second);
    EXPECT_EQ(sphereIntersec.first.value(), X_VEC3D);
    EXPECT_EQ(sphereIntersec.second.value(), -X_VEC3D);
}

TEST(Sphere_Segment3d, Intersection_LHS)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Segment3d seg{Vec3d{-2, 0, 0}, Vec3d{0, 0, 0}};
    auto sphereIntersec = calcIntersection(sphere, seg);
    EXPECT_FALSE(sphereIntersec.first);
    EXPECT_TRUE(sphereIntersec.second);
    EXPECT_EQ(sphereIntersec.first, std::nullopt);
    EXPECT_EQ(sphereIntersec.second.value(), -X_VEC3D);
}

TEST(Sphere_Segment3d, Intersection_RHS)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Segment3d seg{Vec3d{0, 0, 0}, Vec3d{2, 0, 0}};
    auto sphereIntersec = calcIntersection(sphere, seg);
    EXPECT_TRUE(sphereIntersec.first);
    EXPECT_FALSE(sphereIntersec.second);
    EXPECT_EQ(sphereIntersec.first.value(), X_VEC3D);
    EXPECT_EQ(sphereIntersec.second, std::nullopt);
}

TEST(Sphere_Segment3d, Inside_Sphere)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Segment3d seg{Vec3d{-0.5, 0, 0}, Vec3d{0.5, 0, 0}};
    auto sphereIntersec = calcIntersection(sphere, seg);
    EXPECT_FALSE(sphereIntersec.hasIntersection());
}

TEST(Sphere_Segment3d, No_Intersection)
{
    Sphere sphere{ZERO_VEC3D, 1.0};
    Segment3d seg{Vec3d{-2, 2, 0}, Vec3d{2, 2, 0}};
    auto sphereIntersec = calcIntersection(sphere, seg);
    EXPECT_FALSE(sphereIntersec.hasIntersection());
}
