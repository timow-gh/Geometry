

#include <Geometry/Line.hpp>
#include <Geometry/Plane.hpp>
#include <Geometry/Ray.hpp>
#include <LinAl/LinearAlgebra.hpp>
#include <gtest/gtest.h>
#include <Geometry/Segment.hpp>
#include <iostream>

using namespace Geometry;
using namespace LinAl;

TEST(PlaneLine, Parallel_InLine)
{
    Line3d line(Vec3d{0}, Vec3d{1, 0, 0});
    Plane<double_t> plane(Vec3d{0}, Vec3d{0, 0, 1});
    auto intersection = plane.intersection(line);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneLine, Parallel)
{
    Line3d line(Vec3d{1, 0, 0}, Vec3d{1, 0, 0});
    Plane<double_t> plane(Vec3d{0}, Vec3d{0, 0, 1});
    auto intersection = plane.intersection(line);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneLine, Orthogonal)
{
    Line3d line(Vec3d{1, 1, 0}, Vec3d{0, 0, 1});
    Plane<double_t> plane(Vec3d{0}, Vec3d{0, 0, 1});
    auto intersection = plane.intersection(line);
    EXPECT_TRUE(intersection);
    constexpr Vec3d expIntersection{1, 1, 0};
    EXPECT_EQ(intersection, expIntersection);
}

TEST(PlaneLine, Intersection_Origin)
{
    Vec3d lineOrigin = Vec3d{2, 2, 1};
    Vec3d originDir = Vec3d{1, 1, 0} - lineOrigin;
    Line3d line(lineOrigin, originDir);
    Plane<double_t> plane(ZERO_VEC3D, Z_VEC3D);
    auto intersection = plane.intersection(line);
    EXPECT_TRUE(intersection);
    constexpr Vec3d expIntersection{1, 1, 0};
    EXPECT_EQ(intersection, expIntersection);
}

TEST(PlaneLine, viewPlaneBug)
{
    Line3f line(Vec3f{0, 0, 0}, Vec3f{-0.3, 0.2, -1.9});
    Plane<float_t> plane(Vec3f{0, 0, -0.1}, Vec3f{0, 0, 1});
    auto intersection = plane.intersection(line);
    EXPECT_TRUE(intersection);
}

TEST(PlaneSegment3f, Orthogonal)
{
    Plane<float_t> plane(Vec3f{0, 0, 1}, Vec3f{0, 0, 1});
    Segment3f seg{Vec3f{0, 0, -1}, Vec3f{0, 0, 2}};
    auto intersection = plane.intersection(seg);
    EXPECT_TRUE(intersection);
    EXPECT_EQ(intersection.value(), (Vec3f{0, 0, 1}));
}

TEST(PlaneSegment3f, Parallel)
{
    Plane<float_t> plane(Vec3f{0, 0, 1}, Vec3f{0, 0, 1});
    Segment3f seg{Vec3f{0, 0, -1}, Vec3f{1, 0, 0}};
    auto intersection = plane.intersection(seg);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneSegment3f, Parallel_InPlane)
{
    Plane<float_t> plane(Vec3f{0, 0, 1}, Vec3f{0, 0, 1});
    Segment3f seg{Vec3f{0, 0, 1}, Vec3f{1, 0, 0}};
    auto intersection = plane.intersection(seg);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneSegment3f, Touching_Source)
{
    Plane<float_t> plane(Vec3f{0, 0, 0}, Vec3f{0, 0, 1});
    Segment3f seg{Vec3f{2, 2, 1}, Vec3f{1, 1, 0}};
    auto intersection = plane.intersection(seg);
    EXPECT_EQ(intersection, (Vec3f{1, 1, 0}));
}

TEST(PlaneSegment3f, Touching_Target)
{
    Plane<float_t> plane(Vec3f{0, 0, 0}, Vec3f{0, 0, 1});
    Segment3f seg{Vec3f{-2, -2, -1}, Vec3f{-1, -1, 0}};
    auto intersection = plane.intersection(seg);
    EXPECT_EQ(intersection, (Vec3f{-1, -1, 0}));
}

TEST(PlaneRay3d, Touching_Target)
{
    Plane<double_t> plane(Vec3d{0, 0, 0}, Vec3d{0, 0, 1});
    Vec3d raySource = Vec3d{-2, -2, -1};
    Vec3d rayDir = Vec3d{-1, -1, 0} - raySource;
    Ray3d ray{raySource, LinAl::normalize(rayDir)};
    auto intersection = plane.intersection(ray);
    EXPECT_EQ(intersection, (Vec3f{-1, -1, 0}));
}

TEST(PlaneRay3d, Parallel)
{
    Plane<double_t> plane(ZERO_VEC3D, Z_VEC3D);
    Vec3d raySource = ZERO_VEC3D;
    Vec3d rayDir = X_VEC3D - raySource;
    Ray3d ray{raySource, LinAl::normalize(rayDir)};
    auto intersection = plane.intersection(ray);
    EXPECT_EQ(intersection, std::nullopt);
}

