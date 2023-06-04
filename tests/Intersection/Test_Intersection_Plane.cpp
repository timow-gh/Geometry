#include <Geometry/Intersect/IntersectPlane.hpp>
#include <Geometry/Line.hpp>
#include <Geometry/Plane.hpp>
#include <Geometry/Ray.hpp>
#include <Geometry/Segment.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <linal/vec3.hpp>

using namespace Geometry;

TEST(PlaneLine, Parallel_InLine)
{
    Line3d line(linal::vec3d{0}, linal::vec3d{1, 0, 0});
    Plane<double> plane(linal::vec3d{0}, linal::vec3d{0, 0, 1});
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneLine, Parallel)
{
    Line3d line(linal::vec3d{1, 0, 0}, linal::vec3d{1, 0, 0});
    Plane<double> plane(linal::vec3d{0}, linal::vec3d{0, 0, 1});
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneLine, Orthogonal)
{
    Line3d line(linal::vec3d{1, 1, 0}, linal::vec3d{0, 0, 1});
    Plane<double> plane(linal::vec3d{0}, linal::vec3d{0, 0, 1});
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_TRUE(intersection);
    constexpr linal::vec3d expIntersection{1, 1, 0};
    EXPECT_EQ(intersection, expIntersection);
}

TEST(PlaneLine, Intersection_Origin)
{
    linal::vec3d lineOrigin = linal::vec3d{2, 2, 1};
    linal::vec3d originDir = linal::vec3d{1, 1, 0} - lineOrigin;
    Line3d line(lineOrigin, originDir);
    Plane<double> plane(linal::ZERO_VEC3D, linal::Z_VEC3D);
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_TRUE(intersection);
    constexpr linal::vec3d expIntersection{1, 1, 0};
    EXPECT_EQ(intersection, expIntersection);
}

TEST(PlaneLine, viewPlaneBug)
{
    Line3f line(linal::vec3f{0, 0, 0}, linal::vec3f{-0.3, 0.2, -1.9});
    Plane<float> plane(linal::vec3f{0, 0, -0.1}, linal::vec3f{0, 0, 1});
    auto intersection = Geometry::intersect(plane, line);
    EXPECT_TRUE(intersection);
}

TEST(PlaneSegment3f, Orthogonal)
{
    Plane<float> plane(linal::vec3f{0, 0, 1}, linal::vec3f{0, 0, 1});
    Segment3f seg{linal::vec3f{0, 0, -1}, linal::vec3f{0, 0, 2}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_TRUE(intersection);
    EXPECT_EQ(intersection.value(), (linal::vec3f{0, 0, 1}));
}

TEST(PlaneSegment3f, Parallel)
{
    Plane<float> plane(linal::vec3f{0, 0, 1}, linal::vec3f{0, 0, 1});
    Segment3f seg{linal::vec3f{0, 0, -1}, linal::vec3f{1, 0, 0}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneSegment3f, Parallel_InPlane)
{
    Plane<float> plane(linal::vec3f{0, 0, 1}, linal::vec3f{0, 0, 1});
    Segment3f seg{linal::vec3f{0, 0, 1}, linal::vec3f{1, 0, 0}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_EQ(intersection, std::nullopt);
}

TEST(PlaneSegment3f, Touching_Source)
{
    Plane<float> plane(linal::vec3f{0, 0, 0}, linal::vec3f{0, 0, 1});
    Segment3f seg{linal::vec3f{2, 2, 1}, linal::vec3f{1, 1, 0}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_EQ(intersection, (linal::vec3f{1, 1, 0}));
}

TEST(PlaneSegment3f, Touching_Target)
{
    Plane<float> plane(linal::vec3f{0, 0, 0}, linal::vec3f{0, 0, 1});
    Segment3f seg{linal::vec3f{-2, -2, -1}, linal::vec3f{-1, -1, 0}};
    auto intersection = Geometry::intersect(plane, seg);
    EXPECT_EQ(intersection, (linal::vec3f{-1, -1, 0}));
}

TEST(PlaneRay3d, Touching_Target)
{
    Plane<double> plane(linal::vec3d{0, 0, 0}, linal::vec3d{0, 0, 1});
    linal::vec3d raySource = linal::vec3d{-2, -2, -1};
    linal::vec3d rayDir = linal::vec3d{-1, -1, 0} - raySource;
    Ray3d ray{raySource, linal::normalize(rayDir)};
    auto intersection = Geometry::intersect(plane, ray);
    EXPECT_EQ(intersection, (linal::vec3f{-1, -1, 0}));
}

TEST(PlaneRay3d, Parallel)
{
    Plane<double> plane(linal::ZERO_VEC3D, linal::Z_VEC3D);
    linal::vec3d raySource = linal::ZERO_VEC3D;
    linal::vec3d rayDir = linal::X_VEC3D - raySource;
    Ray3d ray{raySource, linal::normalize(rayDir)};
    auto intersection = Geometry::intersect(plane, ray);
    EXPECT_EQ(intersection, std::nullopt);
}
