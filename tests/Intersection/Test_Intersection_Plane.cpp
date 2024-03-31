#include <Geometry/Intersect/IntersectPlane.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Plane.hpp>
#include <Geometry/Ray.hpp>
#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <linal/vec.hpp>

using namespace Geometry;

TEST(PlaneLine, Parallel_InLine)
{
    Line3d line(linal::double3{0}, linal::double3{1, 0, 0});
    Plane<double> plane(linal::double3{0}, linal::double3{0, 0, 1});
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneLine, Parallel)
{
    Line3d line(linal::double3{1, 0, 0}, linal::double3{1, 0, 0});
    Plane<double> plane(linal::double3{0}, linal::double3{0, 0, 1});
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneLine, Orthogonal)
{
    Line3d line(linal::double3{1, 1, 0}, linal::double3{0, 0, 1});
    Plane<double> plane(linal::double3{0}, linal::double3{0, 0, 1});
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_TRUE(intersection);
    constexpr linal::double3 expIntersection{1, 1, 0};
    EXPECT_EQ(intersection, expIntersection);
}

TEST(PlaneLine, Intersection_Origin)
{
    linal::double3 lineOrigin = linal::double3{2, 2, 1};
    linal::double3 originDir = linal::double3{1, 1, 0} - lineOrigin;
    Line3d line(lineOrigin, originDir);
    Plane<double> plane(linal::double3{}, linal::double3Z);
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_TRUE(intersection);
    constexpr linal::double3 expIntersection{1, 1, 0};
    EXPECT_EQ(intersection, expIntersection);
}

TEST(PlaneLine, viewPlaneBug)
{
    Line3f line(linal::float3{0, 0, 0}, linal::float3{-0.3F, 0.2F, -1.9F});
    Plane<float> plane(linal::float3{0, 0, -0.1F}, linal::float3{0, 0, 1});
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_TRUE(intersection);
}

TEST(PlaneSegment3f, Orthogonal)
{
    Plane<float> plane(linal::float3{0, 0, 1}, linal::float3{0, 0, 1});
    Segment3f seg{linal::float3{0, 0, -1}, linal::float3{0, 0, 2}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_TRUE(intersection);
    EXPECT_EQ(intersection.value(), (linal::float3{0, 0, 1}));
}

TEST(PlaneSegment3f, Parallel)
{
    Plane<float> plane(linal::float3{0, 0, 1}, linal::float3{0, 0, 1});
    Segment3f seg{linal::float3{0, 0, -1}, linal::float3{1, 0, 0}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneSegment3f, Parallel_InPlane)
{
    Plane<float> plane(linal::float3{0, 0, 1}, linal::float3{0, 0, 1});
    Segment3f seg{linal::float3{0, 0, 1}, linal::float3{1, 0, 0}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneSegment3f, Touching_Source)
{
    Plane<float> plane(linal::float3{0, 0, 0}, linal::float3{0, 0, 1});
    Segment3f seg{linal::float3{2, 2, 1}, linal::float3{1, 1, 0}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_EQ(intersection, (linal::float3{1, 1, 0}));
}

TEST(PlaneSegment3f, Touching_Target)
{
    Plane<float> plane(linal::float3{0, 0, 0}, linal::float3{0, 0, 1});
    Segment3f seg{linal::float3{-2, -2, -1}, linal::float3{-1, -1, 0}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_EQ(intersection, (linal::float3{-1, -1, 0}));
}

TEST(PlaneRay3d, Touching_Target)
{
    Plane<double> plane(linal::double3{0, 0, 0}, linal::double3{0, 0, 1});
    linal::double3 raySource = linal::double3{-2, -2, -1};
    linal::double3 rayDir = linal::double3{-1, -1, 0} - raySource;
    Ray3d ray{raySource, linal::normalize(rayDir)};
    std::optional<linal::double3> intersection = Geometry::intersect(plane, ray);
    EXPECT_EQ(intersection, (linal::double3{-1, -1, 0}));
}

TEST(PlaneRay3d, Parallel)
{
    Plane<double> plane(linal::double3{}, linal::double3Z);
    linal::double3 raySource = linal::double3{};
    linal::double3 rayDir = linal::double3X - raySource;
    Ray3d ray{raySource, linal::normalize(rayDir)};
    auto intersection = Geometry::intersect(plane, ray);
    EXPECT_EQ(intersection, std::nullopt);
}
